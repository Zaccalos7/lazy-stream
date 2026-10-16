#include "media_job.hpp"
#include "stream_platform.hpp"
#include <algorithm>
#include <cctype>
#include <poll.h>
#include <regex>
#include <sstream>
#include <unistd.h>

namespace orbis {
// Esegue FFmpeg in background e ricompone le righe di avanzamento, perché una lettura della pipe
// può spezzarle.
MediaJob::MediaJob(std::vector<std::string> args, fs::path file) : output(std::move(file)) {
    worker = std::thread([this, args = std::move(args)] {
        try {
            Process p;
            p.start(args);
            std::string pending;
            char buf[4096];
            while (!cancelled) {
                pollfd fd{p.fd(), POLLIN, 0};
                if (poll(&fd, 1, 100) > 0) {
                    auto n = read(p.fd(), buf, sizeof(buf));
                    if (n <= 0) {
                        int result = p.wait();
                        std::lock_guard lock(mutex);
                        if (result != 0)
                            throw std::runtime_error("Transcodifica proxy fallita");
                        status = "READY";
                        return;
                    }
                    pending.append(buf, n);
                    size_t pos;
                    while ((pos = pending.find('\n')) != std::string::npos) {
                        auto line = pending.substr(0, pos);
                        pending.erase(0, pos + 1);
                        if (line.starts_with("out_time_us=")) {
                            try {
                                std::lock_guard lock(mutex);
                                seconds = std::stod(line.substr(12)) / 1000000;
                            } catch (...) {
                            }
                        }
                    }
                }
            }
            p.stop();
            std::lock_guard lock(mutex);
            status = "CANCELLED";
        } catch (const std::exception& e) {
            std::lock_guard lock(mutex);
            status = "ERROR";
            error = e.what();
        }
        if (status != "READY") {
            std::error_code ec;
            fs::remove(output, ec);
        }
    });
}

// Richiede la cancellazione e attende il worker, evitando accessi a memoria già distrutta.
MediaJob::~MediaJob() {
    cancelled = true;
    if (worker.joinable())
        worker.join();
}

// Legge avanzamento ed esito sotto lock per mostrare uno stato coerente della conversione.
Json MediaJob::snapshot() {
    std::lock_guard lock(mutex);
    Json j;
    j.set("status", status);
    j.set("error", error);
    j.set("seconds", seconds);
    j.set("name", output.filename().string());
    return j;
}

// Riusa proxy identificati da percorso, dimensione e data; verifica la cache per scartare
// conversioni interrotte.
Json ProxyLibrary::prepare(const std::string& target) {
    std::lock_guard lock(mutex);
    if (!fs::is_regular_file(target))
        throw std::runtime_error("File sorgente inesistente");
    auto path = fs::canonical(target);
    std::string fingerprint = path.string() + std::to_string(fs::file_size(path)) +
                              std::to_string(fs::last_write_time(path).time_since_epoch().count());
    uint64_t hash = 14695981039346656037ULL;
    for (unsigned char c : fingerprint) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }
    auto name = std::to_string(hash) + ".mp4";
    auto file = root / name;
    if (jobs.contains(name)) {
        auto state = jobs[name]->snapshot();
        if (state.get("status").str() != "ERROR" && state.get("status").str() != "CANCELLED")
            return state;
        jobs.erase(name);
    }
    if (fs::exists(file)) {
        try {
            probe(file.string());
            Json j;
            j.set("status", "READY");
            j.set("name", name);
            return j;
        } catch (...) {
            fs::remove(file);
        }
    }
    auto job = std::make_shared<MediaJob>(
        std::vector<std::string>{ffmpeg(),     "-hide_banner",
                                 "-nostdin",   "-loglevel",
                                 "error",      "-y",
                                 "-i",         path.string(),
                                 "-map",       "0:v:0",
                                 "-map",       "0:a?",
                                 "-vf",        "scale=w='min(1280,iw)':h=-2",
                                 "-c:v",       "libx264",
                                 "-preset",    "veryfast",
                                 "-crf",       "26",
                                 "-c:a",       "aac",
                                 "-b:a",       "96k",
                                 "-movflags",  "+faststart",
                                 "-progress",  "pipe:1",
                                 file.string()},
        file);
    jobs[name] = job;
    return job->snapshot();
}

// Raccoglie gli stati dei job sotto mutex per aggiornare la libreria durante le conversioni.
Json ProxyLibrary::list() {
    std::lock_guard lock(mutex);
    auto out = Json::array();
    for (auto& [name, job] : jobs)
        out.push(job->snapshot());
    return out;
}

// Costruisce player pubblici dove disponibili; la verifica YouTube è indicativa e non richiede
// credenziali dell’account.
Json platformPlayer(const Json& channel) {
    Json out;
    auto platform = channel.get("platform").str();
    if (platform.empty() || platform == "Custom") {
        // Come la pagina Windows, ricava la piattaforma dall'ingest se non è stata selezionata.
        switch (detectStreamPlatform(channel.get("streamUrl").str())) {
        case StreamPlatform::Twitch:
            platform = "Twitch";
            break;
        case StreamPlatform::YouTube:
            platform = "YouTube";
            break;
        case StreamPlatform::Kick:
            platform = "Kick";
            break;
        case StreamPlatform::Facebook:
            platform = "Facebook";
            break;
        case StreamPlatform::TikTok:
            platform = "TikTok";
            break;
        default:
            break;
        }
    }
    // In Windows platformStreamName identifica la piattaforma e channelName il canale pubblico.
    // I primi database Linux usavano invece platformStreamName come handle: lo leggiamo ancora.
    auto name = channel.get("publicName").str();
    if (name.empty()) {
        auto legacy = channel.get("platformStreamName").str();
        auto known = platform;
        std::transform(known.begin(), known.end(), known.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        name = legacy.empty() || legacy == known ? channel.get("channelName").str() : legacy;
    }
    if (name.empty()) {
        out.set("reason", "Nome pubblico o ID piattaforma non configurato");
        return out;
    }
    if (!std::regex_match(name, std::regex("[@A-Za-z0-9_.-]{1,120}")))
        throw std::runtime_error("Nome pubblico non valido");
    if (platform == "Twitch") {
        out.set("embed", "https://player.twitch.tv/?channel=" + name +
                             "&parent=localhost&muted=true&autoplay=true&playsinline=true");
        out.set("publicUrl", "https://www.twitch.tv/" + name);
        out.set("reason", "Il player pubblico verifica lo stato del canale");
    } else if (platform == "YouTube") {
        auto url = name.starts_with("UC")
                       ? "https://www.youtube.com/channel/" + name + "/live"
                       : "https://www.youtube.com/" + (name.starts_with("@") ? name : "@" + name) +
                             "/live";
        out.set("publicUrl", url);
        try {
            auto html = capture(
                {"curl", "--silent", "--show-error", "--location", "--max-time", "8", url}, 10);
            std::smatch m;
            if (html.find("\"isLiveNow\":true") != std::string::npos &&
                std::regex_search(html, m, std::regex("\"videoId\":\"([A-Za-z0-9_-]{11})\""))) {
                out.set("embed",
                        "https://www.youtube.com/embed/" + m[1].str() +
                            "?autoplay=1&mute=1&playsinline=1&origin=http%3A%2F%2Flocalhost");
                out.set("air", "LIVE");
                out.set("reason", "Diretta individuata nella pagina pubblica YouTube");
            } else {
                out.set("air", "UNVERIFIED");
                out.set("reason",
                        "YouTube non ha confermato la diretta; controlla la pagina pubblica");
            }
        } catch (...) {
            out.set("air", "UNVERIFIED");
            out.set("reason", "Pagina pubblica YouTube non raggiungibile");
        }
    } else if (platform == "Kick") {
        out.set("publicUrl", "https://kick.com/" + name);
        out.set("reason", "Apri il canale Kick per verificare la diretta");
    } else if (platform == "TikTok") {
        out.set("publicUrl",
                "https://www.tiktok.com/" + (name.starts_with("@") ? name : "@" + name) + "/live");
        out.set("reason", "Apri la pagina TikTok LIVE");
    } else if (platform == "Facebook") {
        out.set("publicUrl", "https://www.facebook.com/" + name + "/live");
        out.set("reason", "Apri la pagina Facebook Live");
    } else
        out.set("reason", "Player pubblico non disponibile per ingest personalizzato");
    return out;
}

// Misura x264 su un video sintetico breve, scegliendo il preset senza consumare la playlist
// dell’utente.
Json tuneEncoder(const Json& p) {
    validate("profiles", p);
    auto width = std::to_string(int(p.get("videoWidth").num(1280))),
         height = std::to_string(int(p.get("videoHeight").num(720))),
         fps = std::to_string(int(p.get("frameRate").num(30)));
    auto start = std::chrono::steady_clock::now();
    capture({ffmpeg(), "-v", "error", "-f", "lavfi", "-i",
             "testsrc2=s=" + width + "x" + height + ":r=" + fps, "-t", "2", "-an", "-c:v",
             "libx264", "-preset", "veryfast", "-f", "null", "-"},
            30);
    double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    Json result;
    result.set("speed", 2 / elapsed);
    result.set("preset", 2 / elapsed > 2 ? "fast" : (2 / elapsed > 1 ? "veryfast" : "ultrafast"));
    result.set("quality", 2 / elapsed > 2 ? "high" : (2 / elapsed > 1 ? "balanced" : "light"));
    return result;
}
} // namespace orbis
