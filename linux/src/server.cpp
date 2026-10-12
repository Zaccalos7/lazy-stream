#include "core.hpp"
#include "history_cleanup_service.hpp"
#include "live_stream_keys.hpp"
#include "live_rows.hpp"
#include "media_job.hpp"
#include "scene_service.hpp"
#include "version.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <sstream>
#include <sys/socket.h>

namespace orbis {
namespace asio = boost::asio;
namespace http = boost::beast::http;
using tcp = asio::ip::tcp;

namespace {
// Decodifica i parametri URL e controlla le cifre esadecimali prima di usarli come percorsi o
// valori.
std::string decode(const std::string& in) {
    std::string out;
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%' && i + 2 < in.size()) {
            auto hex = in.substr(i + 1, 2);
            if (!std::isxdigit(hex[0]) || !std::isxdigit(hex[1]))
                throw std::runtime_error("URL non valido");
            out += char(std::stoi(hex, nullptr, 16));
            i += 2;
        } else
            out += in[i] == '+' ? ' ' : in[i];
    }
    return out;
}

// Separa e decodifica la query string per centralizzare l’interpretazione dei parametri HTTP.
std::map<std::string, std::string> query(const std::string& target) {
    std::map<std::string, std::string> out;
    auto q = target.find('?');
    if (q == std::string::npos)
        return out;
    std::istringstream s(target.substr(q + 1));
    std::string pair;
    while (std::getline(s, pair, '&')) {
        auto eq = pair.find('=');
        out[decode(pair.substr(0, eq))] =
            eq == std::string::npos ? "" : decode(pair.substr(eq + 1));
    }
    return out;
}

// Associa le estensioni ai tipi HTTP affinché il browser interpreti correttamente le risorse
// locali.
std::string mime(const fs::path& file) {
    auto e = file.extension().string();
    if (e == ".html")
        return "text/html; charset=utf-8";
    if (e == ".js")
        return "text/javascript; charset=utf-8";
    if (e == ".css")
        return "text/css";
    if (e == ".png")
        return "image/png";
    if (e == ".svg")
        return "image/svg+xml";
    if (e == ".webm")
        return "video/webm";
    if (e == ".gif")
        return "image/gif";
    if (e == ".webp")
        return "image/webp";
    if (e == ".jpg")
        return "image/jpeg";
    return "application/octet-stream";
}

// Serve intervalli espliciti bytes=start-end in blocchi fino a 8 MB, consentendo seek senza
// caricare tutto il video.
void mediaResponse(const fs::path& file, const http::request<http::string_body>& req,
                   http::response<http::string_body>& res) {
    uint64_t size = fs::file_size(file), start = 0, end = size ? size - 1 : 0;
    std::string range(req[http::field::range]);
    bool partial = !range.empty();
    if (partial) {
        std::smatch m;
        if (!std::regex_match(range, m, std::regex("bytes=([0-9]+)-([0-9]*)")))
            throw std::runtime_error("Range non supportato");
        start = std::stoull(m[1]);
        if (m[2].length())
            end = std::min<uint64_t>(end, std::stoull(m[2]));
        if (start >= size || end < start) {
            res.result(http::status::range_not_satisfiable);
            res.set(http::field::content_range, "bytes */" + std::to_string(size));
            res.prepare_payload();
            return;
        }
        end = std::min(end, start + 8 * 1024 * 1024 - 1);
        res.result(http::status::partial_content);
        res.set(http::field::content_range, "bytes " + std::to_string(start) + "-" +
                                                std::to_string(end) + "/" + std::to_string(size));
    }
    uint64_t count = size ? end - start + 1 : 0;
    if (count > 65 * 1024 * 1024)
        throw std::runtime_error("Questo media richiede lettura con Range");
    std::ifstream input(file, std::ios::binary);
    input.seekg(start);
    res.body().resize(count);
    input.read(res.body().data(), count);
    if (static_cast<uint64_t>(input.gcount()) != count)
        throw std::runtime_error("Media non leggibile");
    res.set(http::field::accept_ranges, "bytes");
    auto ext = file.extension().string();
    res.set(http::field::content_type, ext == ".mp4" ? "video/mp4" : mime(file));
    res.prepare_payload();
}

// Accetta un solo nome file e rifiuta link simbolici esistenti, limitando i percorsi alle librerie
// previste.
fs::path safeFile(const fs::path& dir, const std::string& name) {
    if (name.empty() || name != fs::path(name).filename() || name == "." || name == "..")
        throw std::runtime_error("Nome file non valido");
    auto file = dir / name;
    if (fs::exists(file) && fs::is_symlink(file))
        throw std::runtime_error("Link non consentito");
    return file;
}

// Salva un media con nome univoco e verifica FFprobe; elimina il file se il contenuto non è
// utilizzabile.
Json upload(Store& store, const std::string& type, const std::string& name,
            const std::string& bytes) {
    if (bytes.empty() || bytes.size() > 64 * 1024 * 1024)
        throw std::runtime_error("File vuoto o oltre 64 MB");
    auto ext = fs::path(name).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".gif" && ext != ".webp" &&
        ext != ".webm" && ext != ".mp4" && ext != ".mov" && ext != ".mkv")
        throw std::runtime_error("Formato media non consentito");
    static std::atomic<long> seq{0};
    std::string file = std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) +
                       "-" + std::to_string(++seq) + ext;
    auto path = safeFile(store.root / type, file);
    writeFile(path, bytes);
    try {
        probe(path.string());
    } catch (...) {
        fs::remove(path);
        throw;
    }
    Json out = success();
    out.set("name", file);
    out.set("path", path.string());
    return out;
}

struct Api {
    Studio& studio;
    const fs::path& web;
    const std::string& token;
    unsigned short port;
    ProxyLibrary proxies{studio.store.root / "proxies"};

    // Autentica prima di instradare le richieste; token e controllo Host proteggono l’accesso al
    // servizio locale.
    http::response<http::string_body> handle(const http::request<http::string_body>& req) {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "OrbisStream Linux");
        res.set(http::field::cache_control, "no-store");
        res.set("X-Content-Type-Options", "nosniff");
        res.set("Referrer-Policy", "no-referrer");
        res.set("Content-Security-Policy",
                "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; img-src "
                "'self' blob: data:; media-src 'self' blob:; frame-src https://player.twitch.tv "
                "https://www.youtube.com; frame-ancestors 'none'; connect-src 'self'");
        std::string target(req.target());
        auto path = decode(target.substr(0, target.find('?')));
        auto q = query(target);
        auto method = req.method();
        auto host = std::string(req[http::field::host]);
        if (host != "127.0.0.1:" + std::to_string(port) &&
            host != "localhost:" + std::to_string(port))
            throw std::runtime_error("Host non consentito");
        if (path == "/" && q["token"] == token) {
            res.result(http::status::see_other);
            res.set(http::field::location, "/");
            res.set(http::field::set_cookie,
                    "orbis-session=" + token + "; HttpOnly; SameSite=Strict; Path=/");
            res.prepare_payload();
            return res;
        }
        bool authorized = req[http::field::authorization] == "Bearer " + token ||
                          std::string(req[http::field::cookie]).find("orbis-session=" + token) !=
                              std::string::npos;
        bool staticFile = path == "/app.js" || path == "/style.css" ||
                          path.starts_with("/resources/") || path.starts_with("/pages/");
        if (!authorized && !staticFile) {
            res.result(http::status::unauthorized);
            res.body() = "Apri l'indirizzo completo stampato dal programma.";
            res.prepare_payload();
            return res;
        }
        if (method != http::verb::get && req["X-Orbis-Token"] != token &&
            req[http::field::authorization] != "Bearer " + token)
            throw std::runtime_error("Token di sessione richiesto");
        if (path == "/" || path.starts_with("/orbis/")) {
            res.set(http::field::content_type, "text/html; charset=utf-8");
            res.body() = readFile(web / "index.html");
            res.prepare_payload();
            return res;
        }
        if (staticFile) {
            fs::path file;
            if (path.starts_with("/resources/") || path.starts_with("/pages/")) {
                const bool resource = path.starts_with("/resources/");
                auto rel = path.substr(resource ? 11 : 7);
                if (rel.empty() || fs::path(rel).is_absolute() ||
                    rel.find("..") != std::string::npos)
                    throw std::runtime_error("Percorso non valido");
                file = web / (resource ? "resources" : "pages") / rel;
            } else
                file = web / path.substr(1);
            res.set(http::field::content_type, mime(file));
            res.body() = readFile(file);
            res.prepare_payload();
            return res;
        }
        Json body;
        if (!req.body().empty() && req[http::field::content_type].starts_with("application/json"))
            body = Json::parse(req.body());
        Json result;
        std::smatch match;
        if (path == "/api/bootstrap") {
            result.set("token", token);
            result.set("version", ORBIS_VERSION);
            result.set("dataDir", studio.store.root.string());
            result.set("sources", sources());
        } else if (path == "/api/state") {
            for (auto k :
                 {"channels", "profiles", "scenes", "buttons", "videos", "history", "preferences"})
                result.set(k, studio.store.list(k));
            auto sessions = studio.live();
            result.set("live", sessions);
            result.set("liveRows", liveRows(studio.store, sessions));
            result.set("metrics", metrics(studio.store.root));
            result.set("cleanupProgress", studio.historyCleanup->progress());
        } else if (path == "/api/live" && method == http::verb::get)
            result = studio.live();
        else if (path == "/api/live/start" && method == http::verb::post) {
            auto id = studio.start(body);
            result = success();
            result.set("id", double(id));
        } else if (path == "/api/live/stop-all" && method == http::verb::post) {
            studio.stopAll();
            result = success();
        } else if (std::regex_match(path, match, std::regex("/api/live/([0-9]+)/([a-z-]+)"))) {
            auto session = studio.session(std::stol(match[1]));
            auto action = match[2].str();
            if (action == "frame" && method == http::verb::get) {
                if (session->done() || !fs::exists(session->frame)) {
                    res.result(http::status::no_content);
                } else {
                    auto stamp = std::to_string(
                        fs::last_write_time(session->frame).time_since_epoch().count());
                    res.set("X-Orbis-Frame", stamp);
                    res.set(http::field::etag, stamp);
                    if (req[http::field::if_none_match] == stamp)
                        res.result(http::status::not_modified);
                    else {
                        res.set(http::field::content_type, "image/jpeg");
                        res.body() = readFile(session->frame);
                    }
                }
                res.prepare_payload();
                return res;
            } else if (method != http::verb::post)
                throw std::runtime_error("Metodo non consentito");
            else if (action == "stop")
                session->stop();
            else if (action == "button") {
                auto b = studio.store.get("buttons", body.get("id").num());
                auto media = safeFile(studio.store.root / "scene-media", b.get("mediaName").str());
                Json scene;
                scene.set("name", b.get("label"));
                auto items = Json::array();
                Json item;
                item.set("sourceKind",
                         b.get("displayMode").str() == "in_scene" ? "OVERLAY" : "FILE");
                item.set("sourceTarget", media.string());
                item.set("audioEnabled", true);
                auto extension = media.extension().string();
                if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
                    extension == ".gif" || extension == ".webp")
                    item.set("loop", true);
                if (b.get("displayMode").str() == "in_scene") {
                    for (auto field : {"x", "y", "width", "height"})
                        if (b.has(field))
                            item.set(field, b.get(field));
                    auto snap = session->snapshot();
                    double w = snap.get("scene").get("width").num(
                               snap.get("profile").get("videoWidth").num(1280)),
                           h = snap.get("scene").get("height").num(
                               snap.get("profile").get("videoHeight").num(720)),
                           iw = b.get("width").num(320), ih = b.get("height").num(180);
                    auto place = b.get("placement").str("custom");
                    if (place != "custom") {
                        item.set("x",
                                 place == "center"
                                     ? (w - iw) / 2
                                     : (place.find("right") != std::string::npos ? w - iw : 0));
                        item.set("y",
                                 place == "center"
                                     ? (h - ih) / 2
                                     : (place.find("bottom") != std::string::npos ? h - ih : 0));
                    }
                }
                items.push(item);
                scene.set("items", items);
                b.set("scene", scene);
                session->command("takeover", b);
            } else
                session->command(action, body);
            result = success();
        } else if (path == "/api/scan" && method == http::verb::post) {
            result = Json::array();
            for (auto& file : scanVideos(body.get("path").str())) {
                Json v;
                v.set("videoPath", file);
                v.set("name", fs::path(file).filename().string());
                v.set("enabled", true);
                v.set("channelId", body.get("channelId"));
                // Il wizard chiede una scansione senza importazione: ripetere l'avvio deve restituire gli stessi file.
                if (body.get("preview").flag()) {
                    result.push(v);
                    continue;
                }
                auto existing = studio.store.list("videos");
                bool duplicate = false;
                for (size_t i = 0; i < existing.size(); ++i)
                    if (existing.at(i).get("videoPath").str() == file &&
                        existing.at(i).get("channelId").num() == body.get("channelId").num())
                        duplicate = true;
                if (!duplicate) {
                    auto id = studio.store.save("videos", v);
                    v.set("id", double(id));
                    result.push(v);
                }
            }
        } else if (path == "/api/probe") {
            result = probe(q["target"]);
        } else if (path == "/api/sources") {
            result = sources();
        } else if (path == "/api/encoders") {
            result.set("text", capture({ffmpeg(), "-hide_banner", "-encoders"}));
        } else if (path == "/api/tune" && method == http::verb::post) {
            result = tuneEncoder(body);
        } else if (path == "/api/proxy" && method == http::verb::post) {
            result = proxies.prepare(body.get("target").str());
        } else if (path == "/api/proxy" && method == http::verb::get) {
            result = proxies.list();
        } else if (std::regex_match(path, match, std::regex("/api/proxy/([0-9]+\\.mp4)"))) {
            mediaResponse(safeFile(studio.store.root / "proxies", match[1]), req, res);
            return res;
        } else if (std::regex_match(path, match, std::regex("/api/video/([0-9]+)/play"))) {
            auto video = studio.store.get("videos", std::stol(match[1]));
            mediaResponse(video.get("videoPath").str(), req, res);
            return res;
        } else if (std::regex_match(path, match, std::regex("/api/platform/([0-9]+)"))) {
            result = platformPlayer(studio.store.get("channels", std::stol(match[1])));
        } else if (path == "/api/update" && method == http::verb::post) {
            auto url = body.get("url").str();
            if (!url.starts_with("https://"))
                throw std::runtime_error("Il manifest aggiornamenti deve usare HTTPS");
            result = Json::parse(capture(
                {"curl", "--fail", "--silent", "--show-error", "--max-time", "10", url}, 12));
            if (result.get("platform").str() != "linux" ||
                !result.get("downloadUrl").str().starts_with("https://"))
                throw std::runtime_error("Manifest Linux non valido");
        } else if (path == "/api/snapshot") {
            std::vector<std::string> args = {ffmpeg(), "-v", "error", "-nostdin"};
            auto kind = q["kind"], target = q["target"];
            if (kind == "SCREEN") {
                if (!target.starts_with(":"))
                    throw std::runtime_error("DISPLAY X11 non valido");
                args.insert(args.end(), {"-f", "x11grab"});
            } else if (kind == "CAMERA") {
                if (!target.starts_with("/dev/video"))
                    throw std::runtime_error("Dispositivo V4L2 non valido");
                args.insert(args.end(), {"-f", "v4l2"});
            } else if (!fs::is_regular_file(target))
                throw std::runtime_error("File inesistente");
            args.insert(args.end(),
                        {"-i", target, "-an", "-frames:v", "1", "-vf", "scale=640:-2", "-c:v",
                         "mjpeg", "-threads", "1", "-f", "image2pipe", "pipe:1"});
            res.set(http::field::content_type, "image/jpeg");
            res.body() = capture(args, 10);
            res.prepare_payload();
            return res;
        } else if (path == "/api/export") {
            result = studio.store.exportData();
            res.set(http::field::content_disposition, "attachment; filename=orbis-linux.json");
        } else if (path == "/api/import" && method == http::verb::post) {
            auto live = studio.live();
            for (size_t i = 0; i < live.size(); ++i)
                if (live.at(i).get("status").str() == "LIVE")
                    throw std::runtime_error("Ferma le dirette prima di importare");
            studio.store.importData(body);
            result = success();
        } else if (path == "/api/history/cleanup/settings" && method == http::verb::post) {
            const double interval = body.get("intervalMonths").num(1),
                         months = body.get("olderThanMonths").num();
            if (!std::isfinite(interval) || !std::isfinite(months) || interval < 1 ||
                interval > 12 || interval != std::floor(interval) || months < -1 || months > 120 ||
                months != std::floor(months))
                throw std::runtime_error("Periodo pulizia non valido");
            historyCleanupThreshold(int(months));
            auto channels = studio.store.list("channels");
            bool saved = false;
            for (size_t i = 0; i < channels.size(); ++i) {
                auto channel = channels.at(i);
                if (!channel.get("isActive").flag(true))
                    continue;
                const bool enabled = body.get("enabled").flag();
                channel.set("autoCleanupEnabled", enabled);
                // Spegnere il toggle conserva i periodi scelti, come SaveAutoCleanup su Windows.
                if (enabled) {
                    channel.set("autoCleanupIntervalMonths", interval);
                    channel.set("autoCleanupOlderThanMonths", months);
                }
                studio.store.save("channels", channel, channel.get("id").num());
                saved = true;
                break;
            }
            if (!saved)
                throw std::runtime_error("Nessuna configurazione canale attiva");
            result = success();
        } else if (path == "/api/history/cleanup" && method == http::verb::get) {
            result = studio.historyCleanup->progress();
        } else if (path == "/api/history/cleanup" && method == http::verb::post) {
            const double months = body.get("months").num();
            if (!std::isfinite(months) || months < -1 || months > 120 ||
                months != std::floor(months))
                throw std::runtime_error("Periodo pulizia non valido");
            result = success();
            if (body.get("async").flag()) {
                const bool accepted =
                    studio.historyCleanup->tryStart(int(months), body.get("channelId").num());
                res.result(accepted ? http::status::accepted : http::status::conflict);
                result = studio.historyCleanup->progress();
            } else
                result.set("deleted",
                           double(studio.store.cleanup(int(months), body.get("channelId").num())));
        } else if (std::regex_match(path, match,
                                    std::regex("/api/media/(overlays|scene-media|images)"))) {
            auto type = match[1].str();
            if (method == http::verb::post)
                result = upload(studio.store, type, q["name"], req.body());
            else {
                result = Json::array();
                for (auto& e : fs::directory_iterator(studio.store.root / type)) {
                    Json v;
                    v.set("name", e.path().filename().string());
                    v.set("path", e.path().string());
                    v.set("bytes", double(e.file_size()));
                    result.push(v);
                }
            }
        } else if (std::regex_match(path, match,
                                    std::regex("/api/media/(overlays|scene-media|images)/(.+)"))) {
            auto file = safeFile(studio.store.root / match[1].str(), match[2]);
            if (method == http::verb::delete_) {
                if (match[1].str() == "overlays" && overlayInUse(studio.store, file))
                    throw std::runtime_error("Overlay usato da una scena o diretta");
                fs::remove(file);
                result = success();
            } else {
                res.set(http::field::content_type, mime(file));
                mediaResponse(file, req, res);
                return res;
            }
        } else if (std::regex_match(path, match, std::regex("/api/logs/([0-9]+)"))) {
            auto file = studio.store.root / "logs" / (match[1].str() + ".log");
            res.set(http::field::content_type, "text/plain; charset=utf-8");
            res.body() = fs::exists(file) ? readFile(file) : "Nessun messaggio FFmpeg";
            res.prepare_payload();
            return res;
        } else if (std::regex_match(
                       path, match,
                       std::regex(
                           "/api/(channels|profiles|scenes|buttons|videos|preferences|history)(?:/"
                           "([0-9]+))?"))) {
            auto k = match[1].str();
            long id = match[2].matched ? std::stol(match[2]) : 0;
            if (method == http::verb::get)
                result = id ? studio.store.get(k, id) : studio.store.list(k);
            else if (method == http::verb::delete_) {
                if (!id)
                    throw std::runtime_error("ID richiesto");
                if (k == "history") {
                    try {
                        if (!studio.session(id)->done())
                            throw std::runtime_error("Diretta ancora attiva");
                    } catch (const std::runtime_error& e) {
                        if (std::string(e.what()) != "Diretta non trovata")
                            throw;
                    }
                    auto record = studio.store.get("history", id);
                    if (!studio.store.removeHistoryIfInactive(id, record.get("startedAt").str()))
                        throw std::runtime_error("Diretta ancora attiva");
                    result = success();
                    res.set(http::field::content_type, "application/json; charset=utf-8");
                    res.body() = result.dump();
                    res.prepare_payload();
                    return res;
                }
                if (k == "profiles" || k == "scenes") {
                    auto channels = studio.store.list("channels");
                    for (size_t i = 0; i < channels.size(); ++i)
                        if (channels.at(i).get(k == "profiles" ? "profileId" : "scenePkid").num() ==
                            id)
                            throw std::runtime_error("Elemento usato da un canale");
                }
                if (k == "scenes")
                    ensureSceneCanDelete(studio.store, id);
                studio.store.remove(k, id);
                result = success();
            } else if (method == http::verb::post || method == http::verb::put) {
                validate(k, body);
                if (k == "channels" && asksForStreamKey(body.get("platform").str(),
                                                        body.get("platformStreamName").str(),
                                                        body.get("streamUrl").str()))
                    body.set("streamKey", standInStreamKey(body.get("channelName").str()));
                if (k == "buttons") {
                    auto all = studio.store.list(k);
                    for (size_t i = 0; i < all.size(); ++i) {
                        auto v = all.at(i);
                        if (v.get("id").num() != id &&
                            (v.get("position").num() == body.get("position").num() ||
                             (!body.get("hotkey").str().empty() &&
                              v.get("hotkey").str() == body.get("hotkey").str())))
                            throw std::runtime_error("Posizione o scorciatoia già occupata");
                    }
                    if (!fs::exists(safeFile(studio.store.root / "scene-media",
                                             body.get("mediaName").str())))
                        throw std::runtime_error("Media pulsante non trovato");
                }
                result = success();
                result.set("id", double(k == "scenes" ? saveScene(studio.store, body, id)
                                                      : studio.store.save(k, body, id)));
            } else
                throw std::runtime_error("Metodo non consentito");
        } else {
            res.result(http::status::not_found);
            result.set("response", "error");
            result.set("message", "Route non trovata");
        }
        res.set(http::field::content_type, "application/json; charset=utf-8");
        res.body() = result.dump();
        res.prepare_payload();
        return res;
    }
};
} // namespace

// Ascolta solo su loopback e distribuisce le connessioni su un pool, evitando che una richiesta
// blocchi tutte le altre.
void serve(Studio& studio, unsigned short port, const fs::path& web, const std::string& token) {
    asio::io_context io;
    tcp::acceptor acceptor(io, {asio::ip::make_address("127.0.0.1"), port});
    asio::thread_pool pool(8);
    Api api{studio, web, token, port};
    for (;;) {
        tcp::socket socket(io);
        acceptor.accept(socket);
        asio::post(pool, [socket = std::move(socket), &api]() mutable {
            try {
                boost::beast::tcp_stream stream(std::move(socket));
                stream.expires_after(std::chrono::seconds(30));
                boost::beast::flat_buffer buffer;
                http::request_parser<http::string_body> parser;
                parser.body_limit(65 * 1024 * 1024);
                http::read(stream, buffer, parser);
                auto req = parser.release();
                http::response<http::string_body> res;
                try {
                    res = api.handle(req);
                } catch (const std::exception& e) {
                    res = {http::status::bad_request, req.version()};
                    res.set(http::field::content_type, "application/json");
                    Json j;
                    j.set("response", "error");
                    j.set("message", e.what());
                    res.body() = j.dump();
                    res.prepare_payload();
                }
                res.keep_alive(false);
                http::write(stream, res);
                boost::system::error_code ec;
                stream.socket().shutdown(tcp::socket::shutdown_both, ec);
            } catch (...) {
            }
        });
    }
}
} // namespace orbis
