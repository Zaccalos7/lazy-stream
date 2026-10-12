#include "core.hpp"
#include "live_stream_keys.hpp"
#include "stream_platform.hpp"
#include <algorithm>
#include <cmath>
#include <fcntl.h>
#include <fstream>
#include <poll.h>
#include <signal.h>
#include <sstream>
#include <unistd.h>

namespace orbis {
namespace {
// Serializza numeri per i filtri FFmpeg senza dipendere dalla virgola decimale della lingua di
// sistema.
std::string number(double n) {
    std::ostringstream s;
    s << n;
    return s.str();
}

// Accoda argomenti distinti, mantenendo i parametri FFmpeg separati senza quoting da shell.
void add(std::vector<std::string>& a, std::initializer_list<std::string> b) {
    a.insert(a.end(), b);
}

// Normalizza il tipo della sorgente per condividere la costruzione dei filtri tra scene e playlist.
std::string kind(const Json& item) {
    auto s = item.get("sourceKind").str("FILE");
    if (s == "0")
        return "FILE";
    if (s == "1")
        return "SCREEN";
    if (s == "2")
        return "CAMERA";
    if (s == "3")
        return "MICROPHONE";
    if (s == "4")
        return "OVERLAY";
    return s;
}

// Riconosce le immagini statiche, che richiedono ripetizione dei frame invece di riproduzione
// finita.
bool still(const std::string& file) {
    auto e = fs::path(file).extension().string();
    std::transform(e.begin(), e.end(), e.begin(), ::tolower);
    return e == ".png" || e == ".jpg" || e == ".jpeg" || e == ".bmp";
}

// Verifica gli stream rilevati da FFprobe prima di collegare un ingresso al mixer.
bool hasAudio(const Json& p) {
    auto streams = p.get("streams");
    for (size_t i = 0; i < streams.size(); ++i)
        if (streams.at(i).get("codec_type").str() == "audio")
            return true;
    return false;
}

// Trasforma un file in una scena, così la playlist usa lo stesso motore di composizione delle
// scene.
Json fileScene(const std::string& file) {
    Json s;
    s.set("name", fs::path(file).filename().string());
    auto items = Json::array();
    Json item;
    item.set("sourceKind", "FILE");
    item.set("sourceTarget", file);
    item.set("audioEnabled", true);
    item.set("volume", 100.0);
    items.push(item);
    s.set("items", items);
    return s;
}

// Legge il formato a 24 bit big-endian dei tag FLV senza dipendere dall’endianness della CPU.
uint32_t u24(const unsigned char* p) {
    return (uint32_t(p[0]) << 16) | (uint32_t(p[1]) << 8) | p[2];
}

// Scrive i campi FLV a 24 bit byte per byte per conservare il formato di rete.
void put24(unsigned char* p, uint32_t n) {
    p[0] = n >> 16;
    p[1] = n >> 8;
    p[2] = n;
}
} // namespace

// Costruisce ingressi e filtri FFmpeg per video, audio e anteprima; una pipeline comune mantiene
// coerente l’uscita.
Build buildProducer(const Json& scene, const Json& profile, double offset,
                    const fs::path& preview) {
    Build result;
    auto& a = result.args;
    a = {ffmpeg(), "-hide_banner", "-nostdin", "-loglevel", "warning", "-filter_complex_threads",
         "1",      "-threads",     "2",        "-y"};
    int w = profile.get("videoWidth").num(scene.get("width").num(1280)),
        h = profile.get("videoHeight").num(scene.get("height").num(720));
    double fps = profile.get("frameRate").num(30);
    if (w < 16 || h < 16 || w > 7680 || h > 7680 || w % 2 || h % 2 || fps < 1 || fps > 120)
        throw std::runtime_error("Canvas o FPS non validi");
    double sw = scene.get("width").num(w), sh = scene.get("height").num(h);
    if (sw <= 0 || sh <= 0)
        throw std::runtime_error("Canvas non valido");
    double fit = std::min(w / sw, h / sh), padX = (w - sw * fit) / 2, padY = (h - sh * fit) / 2;
    auto items = scene.get("items").clone();
    if (items.size() == 0)
        throw std::runtime_error("La scena non ha sorgenti");
    for (size_t i = 0; i < items.size(); ++i) {
        auto item = items.at(i);
        item.set("x", std::round(item.get("x").num() * fit + padX));
        item.set("y", std::round(item.get("y").num() * fit + padY));
        item.set("width", std::max(1.0, std::round(item.get("width").num(sw) * fit)));
        item.set("height", std::max(1.0, std::round(item.get("height").num(sh) * fit)));
    }
    std::string graph;
    std::vector<int> audio;
    std::vector<double> volumes;
    std::vector<bool> picture;
    int input = 0;
    bool finite = false, capturePresent = false;
    double duration = 0;
    for (size_t i = 0; i < items.size(); ++i) {
        auto t = items.at(i);
        auto target = t.get("sourceTarget").str();
        auto k = kind(t);
        if (target.empty()) {
            picture.push_back(false);
            continue;
        }
        bool au = false, vi = k != "MICROPHONE";
        if (k == "SCREEN" || k == "CAMERA" || k == "MICROPHONE")
            capturePresent = true;
        if (k == "SCREEN") {
            if (!target.starts_with(":"))
                throw std::runtime_error("Cattura schermo: usare DISPLAY X11, ad esempio :0.0");
            add(a, {"-thread_queue_size", "512", "-f", "x11grab", "-framerate", number(fps), "-i",
                    target});
        } else if (k == "CAMERA") {
            if (!target.starts_with("/dev/video"))
                throw std::runtime_error("Dispositivo V4L2 non valido");
            add(a, {"-thread_queue_size", "512", "-f", "v4l2", "-framerate", number(fps), "-i",
                    target});
        } else if (k == "MICROPHONE") {
            add(a, {"-thread_queue_size", "512", "-f", "pulse", "-i", target});
            au = true;
        } else if (k == "FILE" || k == "OVERLAY") {
            auto p = probe(target);
            au = hasAudio(p);
            if (still(target)) {
                add(a, {"-loop", "1", "-framerate", number(fps)});
            } else {
                if (k == "OVERLAY" || t.get("loop").flag())
                    add(a, {"-stream_loop", "-1"});
                if (offset > 0 && k == "FILE")
                    add(a, {"-ss", number(offset)});
            }
            add(a, {"-re", "-i", target});
            double d = p.get("format").get("duration").num();
            if (k == "FILE" && !still(target) && !t.get("loop").flag() && d > 0) {
                duration = std::max(duration, d - offset);
                finite = true;
            }
        } else
            throw std::runtime_error("Tipo sorgente non supportato");
        picture.push_back(vi);
        if (vi) {
            int tw = t.get("width").num(w), th = t.get("height").num(h);
            if (tw < 1 || th < 1 || tw > 7680 || th > 7680)
                throw std::runtime_error("Dimensioni tile non valide");
            graph +=
                "[" + std::to_string(input) + ":v]setpts=PTS-STARTPTS,scale=" + std::to_string(tw) +
                ":" + std::to_string(th) +
                ":force_original_aspect_ratio=decrease,pad=" + std::to_string(tw) + ":" +
                std::to_string(th) + ":(ow-iw)/2:(oh-ih)/2:color=black@0,setsar=1,format=rgba[v" +
                std::to_string(i) + "];";
        }
        if (au && k != "OVERLAY" && t.get("audioEnabled").flag(true)) {
            audio.push_back(input);
            volumes.push_back(std::clamp(t.get("volume").num(100), 0.0, 200.0) / 100);
        }
        ++input;
    }
    add(a,
        {"-f", "lavfi", "-i",
         "color=c=black:s=" + std::to_string(w) + "x" + std::to_string(h) + ":r=" + number(fps)});
    int background = input++;
    graph += "[" + std::to_string(background) + ":v]setpts=PTS-STARTPTS[base0];";
    int layer = 0;
    for (size_t i = 0; i < items.size(); ++i)
        if (picture[i]) {
            auto t = items.at(i);
            graph += "[base" + std::to_string(layer) + "][v" + std::to_string(i) +
                     "]overlay=x=" + number(t.get("x").num()) + ":y=" + number(t.get("y").num()) +
                     ":eof_action=pass:shortest=0[base" + std::to_string(layer + 1) + "];";
            ++layer;
        }
    graph += "[base" + std::to_string(layer) + "]fps=" + number(fps) + ",format=yuv420p";
    if (!preview.empty())
        graph += ",split=2[outv][preview];[preview]fps=2,scale=640:-2[thumb];";
    else
        graph += "[outv];";
    if (audio.empty()) {
        add(a, {"-f", "lavfi", "-i", "anullsrc=r=48000:cl=stereo"});
        graph += "[" + std::to_string(input) + ":a]anull[outa]";
    } else {
        for (size_t i = 0; i < audio.size(); ++i)
            graph +=
                "[" + std::to_string(audio[i]) +
                ":a]asetpts=PTS-STARTPTS,aresample=48000,aformat=channel_layouts=stereo,volume=" +
                number(volumes[i]) + "[a" + std::to_string(i) + "];";
        for (size_t i = 0; i < audio.size(); ++i)
            graph += "[a" + std::to_string(i) + "]";
        graph +=
            "amix=inputs=" + std::to_string(audio.size()) + ":duration=longest:normalize=0[outa]";
    }
    add(a, {"-filter_complex", graph, "-map", "[outv]", "-map", "[outa]"});
    auto codec = profile.get("videoCodecName").str("libx264");
    if (codec != "libx264" && codec != "h264_nvenc" && codec != "h264_qsv" && codec != "h264_vaapi")
        throw std::runtime_error("RTMP richiede un encoder H.264");
    if (codec == "h264_vaapi")
        throw std::runtime_error(
            "VAAPI richiede un percorso hardware dedicato, usare x264/NVENC/QSV");
    auto bitrate = number(profile.get("videoBitrate").num(4500)) + "k";
    add(a, {"-c:v", codec, "-b:v", bitrate, "-maxrate", bitrate, "-bufsize",
            number(profile.get("videoBitrate").num(4500) * 2) + "k", "-g",
            number(std::round(profile.get("gopSize").num(2) * fps)), "-pix_fmt", "yuv420p"});
    if (codec == "libx264") {
        add(a, {"-preset", profile.get("preset").str("veryfast")});
        if (profile.get("lowLatency").flag(true))
            add(a, {"-tune", "zerolatency"});
    }
    auto options = profile.get("videoSettingsOptions");
    for (size_t i = 0; i < options.size(); ++i) {
        auto o = options.at(i);
        auto key = o.get("key").str();
        if (key == "crf" || key == "profile:v" || key == "level" || key == "bf" || key == "refs")
            add(a, {"-" + key, o.get("value").str()});
        else
            throw std::runtime_error("Opzione FFmpeg non consentita: " + key);
    }
    add(a, {"-c:a", "aac", "-b:a", number(profile.get("audioBitrate").num(128)) + "k", "-ar",
            "48000", "-ac", "2"});
    if (capturePresent)
        finite = false;
    if (finite) {
        result.duration = std::max(0.05, duration);
        add(a, {"-t", number(result.duration)});
    }
    add(a, {"-flvflags", "no_duration_filesize", "-f", "flv", "pipe:1"});
    if (!preview.empty()) {
        add(a, {"-map", "[thumb]", "-an", "-c:v", "mjpeg", "-q:v", "5", "-threads", "1"});
        if (finite)
            add(a, {"-t", number(result.duration)});
        add(a, {"-f", "image2", "-update", "1", "-atomic_writing", "1", preview.string()});
    }
    return result;
}

// Azzera il parser del nuovo produttore conservando l’epoca di uscita, così i timestamp restano
// continui.
void FlvRelay::segment() {
    pending.clear();
    header = false;
    firstTag = true;
    base = sentHeader ? last + 1 : 0;
    if (!sentHeader)
        epoch = std::chrono::steady_clock::now();
}

// Ricompone tag FLV anche da letture parziali e riscrive i timestamp; invia una sola intestazione
// al publisher.
std::vector<unsigned char> FlvRelay::feed(const unsigned char* bytes, size_t size, bool pace) {
    pending.insert(pending.end(), bytes, bytes + size);
    if (pending.size() > 32 * 1024 * 1024)
        throw std::runtime_error("FLV oltre il limite");
    std::vector<unsigned char> out;
    size_t used = 0;
    if (!header) {
        if (pending.size() < 13)
            return out;
        if (pending[0] != 'F' || pending[1] != 'L' || pending[2] != 'V')
            throw std::runtime_error("Header FLV non valido");
        uint32_t offset = (uint32_t(pending[5]) << 24) | (uint32_t(pending[6]) << 16) |
                          (uint32_t(pending[7]) << 8) | pending[8];
        if (offset < 9 || offset > 1024)
            throw std::runtime_error("Offset FLV non valido");
        if (pending.size() < offset + 4)
            return out;
        if (!sentHeader) {
            out.insert(out.end(), pending.begin(), pending.begin() + offset + 4);
            sentHeader = true;
        }
        used = offset + 4;
        header = true;
    }
    while (pending.size() - used >= 15) {
        auto* p = pending.data() + used;
        uint32_t len = u24(p + 1);
        if (len > 16 * 1024 * 1024)
            throw std::runtime_error("Tag FLV troppo grande");
        if (pending.size() - used < len + 15)
            break;
        uint32_t previous = (uint32_t(p[11 + len]) << 24) | (uint32_t(p[12 + len]) << 16) |
                            (uint32_t(p[13 + len]) << 8) | p[14 + len];
        if (previous != len + 11)
            throw std::runtime_error("Dimensione FLV non valida");
        if (p[0] != 8 && p[0] != 9 && p[0] != 18)
            throw std::runtime_error("Tipo tag FLV non valido");
        uint32_t ts = u24(p + 4) | (uint32_t(p[7]) << 24);
        if (firstTag && (p[0] == 8 || p[0] == 9)) {
            first = ts;
            firstTag = false;
        }
        uint64_t normalized = firstTag ? base : base + (ts >= first ? ts - first : 0);
        normalized = std::max(last, normalized);
        last = normalized;
        put24(p + 4, uint32_t(normalized));
        p[7] = normalized >> 24;
        out.insert(out.end(), p, p + len + 15);
        used += len + 15;
    }
    pending.erase(pending.begin(), pending.begin() + used);
    if (pace && last) {
        auto target = epoch + std::chrono::milliseconds(last);
        auto current = std::chrono::steady_clock::now();
        if (target > current)
            std::this_thread::sleep_for(
                std::min(target - current,
                         std::chrono::steady_clock::duration(std::chrono::milliseconds(250))));
    }
    return out;
}

// Prepara gli snapshot della diretta per isolare il worker dalle successive modifiche alla
// configurazione.
Session::Session(Store& s, Json r, long pk)
    : store(s), request(r.clone()), id(pk), channel(r.get("channelId").num()) {
    profile = r.get("profile").clone();
    profile.set("targetBitrate", profile.get("videoBitrate"));
    frame = store.root / "previews" / (std::to_string(id) + ".jpg");
    if (r.has("scene")) {
        original = r.get("scene").clone();
        playlist.push_back(original);
    } else {
        auto files = r.get("files");
        for (size_t i = 0; i < files.size(); ++i) {
            playlist.push_back(fileScene(files.at(i).str()));
            long profileId = r.get("videoProfiles").at(i).num();
            auto vp = profileId > 0 ? store.get("profiles", profileId) : profile.clone();
            vp.set("targetBitrate", vp.get("videoBitrate"));
            videoProfiles.push_back(vp);
        }
        if (playlist.empty())
            throw std::runtime_error("Playlist vuota");
        original = playlist[0];
    }
    offset = r.get("offsetSeconds").num();
}

// Ferma e attende il worker prima di distruggere i dati a cui il thread accede.
Session::~Session() {
    stop();
}

// Sposta la codifica sul worker, lasciando il server disponibile per anteprime e comandi.
void Session::start() {
    worker = std::thread([this] { run(); });
}

// Segnala l’arresto e serializza il join: più richieste di stop non devono attendere lo stesso
// thread insieme.
void Session::stop() {
    std::lock_guard lock(stopMutex);
    stopped = true;
    if (worker.joinable())
        worker.join();
}

// Copia lo stato sotto lock per offrire al browser una lettura coerente mentre il worker avanza.
Json Session::snapshot() {
    std::lock_guard lock(mutex);
    Json s;
    s.set("id", double(id));
    s.set("channelId", double(channel));
    s.set("status", state);
    s.set("error", error);
    s.set("position", double(index));
    s.set("total", double(playlist.size()));
    s.set("offsetSeconds", offset);
    s.set("scene", original);
    s.set("takeover", takeover);
    s.set("producerPid", double(producerPid));
    s.set("publisherPid", double(publisherPid));
    if (producerPid > 0)
        s.set("producerMetrics", processMetrics(producerPid));
    if (publisherPid > 0)
        s.set("publisherMetrics", processMetrics(publisherPid));
    s.set("profile", profile);
    s.set("outputKbps", outputKbps);
    s.set("reconnectAttempts", double(reconnectAttempts));
    return s;
}

// Aggiorna i comandi sotto mutex e segnala il cambio al worker, evitando interventi concorrenti sui
// processi.
void Session::command(const std::string& name, const Json& data) {
    std::lock_guard lock(mutex);
    if (finished)
        throw std::runtime_error("Diretta terminata");
    if (name == "next") {
        index = (index + 1) % playlist.size();
        offset = 0;
        original = playlist[index];
        takeover = Json();
    } else if (name == "jump") {
        int target = data.get("index").num(-1);
        if (target < 0 || target >= int(playlist.size()))
            throw std::runtime_error("Indice non valido");
        index = target;
        offset = 0;
        original = playlist[index];
        takeover = Json();
    } else if (name == "resume")
        takeover = Json();
    else if (name == "takeover")
        takeover = data.clone();
    else if (name == "spot") {
        auto items = original.get("items");
        auto ext = fs::path(items.at(0).get("sourceTarget").str()).extension().string();
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" || ext == ".webp") {
            takeover = data.clone();
            changed = true;
            return;
        }
        spots.push_back(data.clone());
        return;
    } else if (name == "parameters") {
        validate("profiles", data);
        profile = data.clone();
        profile.set("targetBitrate", profile.get("videoBitrate"));
        if (!videoProfiles.empty())
            videoProfiles[index] = profile.clone();
    } else if (name == "volume") {
        int i = data.get("index").num(-1);
        auto items = original.get("items");
        if (i < 0 || i >= int(items.size()))
            throw std::runtime_error("Sorgente non trovata");
        items.at(i).set("volume", std::clamp(data.get("volume").num(100), 0.0, 200.0));
        items.at(i).set("audioEnabled", data.get("audioEnabled").flag(true));
    } else
        throw std::runtime_error("Comando sconosciuto");
    changed = true;
}

// Mantiene il publisher aperto mentre sostituisce i produttori; gestisce backpressure, watchdog e
// tentativi di riconnessione.
void Session::run() {
    int relayPipe[2]{-1, -1};
    Process publisher, producer;
    FlvRelay relay;
    try {
        std::string output = request.get("outputFile").str();
        if (output.empty()) {
            auto url = request.get("streamUrl").str();
            auto key = request.get("streamKey").str();
            if ((!url.starts_with("rtmp://") && !url.starts_with("rtmps://")) || key.empty() ||
                isStandInStreamKey(key))
                throw std::runtime_error("Server e chiave richiesti");
            output = normalizeIngestUrl(url + (url.ends_with('/') ? "" : "/") + key);
        }
        if (pipe2(relayPipe, O_CLOEXEC))
            throw std::runtime_error("Relay pipe fallita");
        publisher.start({ffmpeg(), "-hide_banner", "-nostdin", "-loglevel", "error", "-y", "-f",
                         "flv", "-i", "pipe:0", "-c", "copy", "-flvflags", "no_duration_filesize",
                         "-f", "flv", output},
                        relayPipe[0], {}, false);
        close(relayPipe[0]);
        relayPipe[0] = -1;
        fcntl(relayPipe[1], F_SETFL, O_NONBLOCK);
        {
            std::lock_guard lock(mutex);
            publisherPid = publisher.id();
            state = "LIVE";
        }
        auto relayEpoch = std::chrono::steady_clock::now();
        auto lastAdapt = relayEpoch;
        size_t sentBytes = 0;
        int stalledPolls = 0;
        auto h = store.get("history", id);
        h.set("status", "LIVE");
        store.save("history", h, id);
        while (!stopped) {
            Json scene, params;
            bool isTakeover, inScene = false;
            double startOffset;
            int segmentIndex;
            {
                std::lock_guard lock(mutex);
                changed = false;
                scene = original.clone();
                params = videoProfiles.empty() ? profile.clone() : videoProfiles[index].clone();
                isTakeover = takeover.has("scene");
                if (isTakeover) {
                    auto b = takeover.get("scene").clone();
                    if (takeover.get("displayMode").str() == "in_scene") {
                        inScene = true;
                        auto items = scene.get("items");
                        auto extra = b.get("items");
                        for (size_t i = 0; i < extra.size(); ++i)
                            items.push(extra.at(i));
                    } else
                        scene = b;
                }
                startOffset = isTakeover && !inScene ? 0 : offset;
                segmentIndex = index;
            }
            auto build = buildProducer(scene, params, startOffset, frame);
            double takeoverDuration = 0;
            if (isTakeover) {
                std::lock_guard lock(mutex);
                takeoverDuration = takeover.get("durationSeconds").num();
            }
            producer.start(build.args, -1, store.root / "logs" / (std::to_string(id) + ".log"));
            {
                std::lock_guard lock(mutex);
                producerPid = producer.id();
            }
            relay.segment();
            auto mediaStart = relay.timestamp();
            auto lastByte = std::chrono::steady_clock::now();
            bool received = false;
            unsigned char bytes[32768];
            while (!stopped && !changed) {
                pollfd rd{producer.fd(), POLLIN, 0};
                int n = poll(&rd, 1, 100);
                if (n < 0 && errno == EINTR)
                    continue;
                if (n > 0) {
                    auto count = read(producer.fd(), bytes, sizeof(bytes));
                    if (count <= 0)
                        break;
                    received = true;
                    lastByte = std::chrono::steady_clock::now();
                    auto tags = relay.feed(bytes, count);
                    size_t pos = 0;
                    while (pos < tags.size() && !stopped) {
                        pollfd wr{relayPipe[1], POLLOUT, 0};
                        int ready = poll(&wr, 1, 100);
                        if (ready == 0)
                            ++stalledPolls;
                        if (ready > 0) {
                            if (wr.revents & (POLLERR | POLLHUP))
                                throw std::runtime_error("Connessione di uscita interrotta");
                            auto written =
                                write(relayPipe[1], tags.data() + pos, tags.size() - pos);
                            if (written > 0) {
                                pos += written;
                                sentBytes += written;
                                double seconds = std::chrono::duration<double>(
                                                     std::chrono::steady_clock::now() - relayEpoch)
                                                     .count();
                                std::lock_guard lock(mutex);
                                outputKbps = seconds > 0 ? sentBytes * 8.0 / seconds / 1000 : 0;
                            } else if (errno != EAGAIN && errno != EINTR)
                                throw std::runtime_error("Publisher non disponibile");
                        }
                        if (!publisher.running())
                            throw std::runtime_error("Publisher terminato");
                        if (std::chrono::steady_clock::now() - lastByte > std::chrono::seconds(30))
                            throw std::runtime_error("Uscita bloccata da 30 secondi");
                    }
                    if (!isTakeover || inScene) {
                        std::lock_guard lock(mutex);
                        if (index == segmentIndex)
                            offset = startOffset + double(relay.timestamp() - mediaStart) / 1000;
                    }
                    if (stalledPolls >= 20 &&
                        std::chrono::steady_clock::now() - lastAdapt > std::chrono::seconds(10)) {
                        std::lock_guard lock(mutex);
                        if (params.get("adaptiveBitrate").flag()) {
                            double rate =
                                std::max(300.0, params.get("videoBitrate").num(4500) * 0.75);
                            profile.set("videoBitrate", rate);
                            if (!videoProfiles.empty())
                                videoProfiles[index].set("videoBitrate", rate);
                            changed = true;
                        }
                        stalledPolls = 0;
                        lastAdapt = std::chrono::steady_clock::now();
                    }
                    if (stalledPolls == 0 && params.get("adaptiveBitrate").flag() &&
                        std::chrono::steady_clock::now() - lastAdapt > std::chrono::seconds(30) &&
                        params.get("videoBitrate").num() < params.get("targetBitrate").num()) {
                        std::lock_guard lock(mutex);
                        double rate = std::min(params.get("targetBitrate").num(),
                                               params.get("videoBitrate").num() * 1.15);
                        profile.set("videoBitrate", rate);
                        if (!videoProfiles.empty())
                            videoProfiles[index].set("videoBitrate", rate);
                        changed = true;
                        lastAdapt = std::chrono::steady_clock::now();
                    }
                }
                if (isTakeover && takeoverDuration > 0 &&
                    relay.timestamp() - mediaStart >= takeoverDuration * 1000) {
                    producer.stop();
                    break;
                }
                if (std::chrono::steady_clock::now() - lastByte > std::chrono::seconds(30))
                    throw std::runtime_error("FFmpeg non produce dati da 30 secondi");
                if (!publisher.running())
                    throw std::runtime_error("Publisher terminato");
            }
            bool interrupted = changed || stopped;
            int result = interrupted ? (producer.stop(), 0) : producer.wait();
            if (!interrupted && (!received || result != 0))
                throw std::runtime_error("Codifica fallita; consulta il registro della diretta");
            if (interrupted)
                continue;
            {
                std::lock_guard lock(mutex);
                if (isTakeover) {
                    takeover = Json();
                    if (inScene && build.duration > 0 &&
                        offset - startOffset >= build.duration - 0.2) {
                        offset = 0;
                        ++index;
                        if (index >= int(playlist.size())) {
                            if (request.get("loop").flag())
                                index = 0;
                            else
                                break;
                        }
                        original = playlist[index];
                    }
                } else if (!spots.empty()) {
                    takeover = spots.front();
                    spots.erase(spots.begin());
                    offset = 0;
                    index = (index + 1) % playlist.size();
                    original = playlist[index];
                } else {
                    offset = 0;
                    ++index;
                    if (index >= int(playlist.size())) {
                        if (request.get("loop").flag())
                            index = 0;
                        else
                            break;
                    }
                    original = playlist[index];
                }
            }
        }
        producer.stop();
        if (relayPipe[1] >= 0) {
            close(relayPipe[1]);
            relayPipe[1] = -1;
        }
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (publisher.running() && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        publisher.stop();
        {
            std::lock_guard lock(mutex);
            state = "ENDED";
        }
    } catch (const std::exception& e) {
        std::lock_guard lock(mutex);
        state = stopped ? "ENDED" : "ERROR";
        error = e.what();
    }
    producer.stop();
    publisher.stop();
    for (auto fd : relayPipe)
        if (fd >= 0)
            close(fd);
    bool retry = false;
    {
        std::lock_guard lock(mutex);
        retry = state == "ERROR" && !stopped && request.get("reconnect").flag(true) &&
                reconnectAttempts < 3;
        if (retry) {
            ++reconnectAttempts;
            state = "RECONNECTING";
        }
    }
    if (retry) {
        for (int i = 0; i < 20 && !stopped; ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!stopped) {
            run();
            return;
        }
    }
    {
        std::lock_guard lock(mutex);
        producerPid = publisherPid = -1;
    }
    try {
        auto h = store.get("history", id);
        auto s = snapshot();
        h.set("status", s.get("status"));
        h.set("error", s.get("error"));
        h.set("offsetSeconds", s.get("offsetSeconds"));
        h.set("endedAt", now());
        store.save("history", h, id);
    } catch (...) {
    }
    finished = true;
}

// Avvia la manutenzione periodica separatamente dalle richieste, per applicare la conservazione di
// storico e log.
Studio::Studio(const fs::path& root) : store(root) {
    housekeeping = std::jthread([this](std::stop_token stop) {
        while (!stop.stop_requested()) {
            try {
                auto channels = store.list("channels");
                for (size_t i = 0; i < channels.size(); ++i) {
                    auto c = channels.at(i);
                    int months = c.get("autoCleanupOlderThanMonths").num(),
                        interval = c.get("autoCleanupIntervalMonths").num(1);
                    if (months > 0 && interval > 0) {
                        auto last = c.get("lastCleanupEpoch").num();
                        double epoch = std::time(nullptr);
                        if (epoch - last > interval * 30.0 * 86400) {
                            store.cleanup(months, c.get("id").num());
                            c.set("lastCleanupEpoch", epoch);
                            store.save("channels", c, c.get("id").num());
                        }
                    }
                }
                for (auto& e : fs::directory_iterator(store.root / "logs")) {
                    auto age = fs::file_time_type::clock::now() - e.last_write_time();
                    if (age > std::chrono::hours(72))
                        fs::remove(e.path());
                }
            } catch (...) {
            }
            for (int i = 0; i < 60 && !stop.stop_requested(); ++i)
                std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
}

// Interrompe la manutenzione e le sessioni prima che venga distrutto lo store condiviso.
Studio::~Studio() {
    housekeeping.request_stop();
    housekeeping.join();
    stopAll();
}

// Valida e completa la richiesta prima di creare la sessione, così gli errori emergono prima
// dell’avvio della diretta.
long Studio::start(Json r) {
    std::lock_guard lock(mutex);
    for (auto it = sessions.begin(); it != sessions.end();)
        if (it->second->done())
            it = sessions.erase(it);
        else
            ++it;
    long channel = r.get("channelId").num();
    if (channel) {
        auto c = store.get("channels", channel);
        if (!c.get("isActive").flag(true))
            throw std::runtime_error("Canale disabilitato");
        for (auto& [id, s] : sessions)
            if (s->channel == channel && !s->done())
                throw std::runtime_error("Canale già in diretta");
        r.set("streamUrl", c.get("streamUrl"));
        if (!asksForStreamKey(c.get("platform").str(), c.get("platformStreamName").str(),
                              c.get("streamUrl").str()))
            r.set("streamKey", c.get("streamKey"));
        if (!r.has("files") && !r.has("scene")) {
            if (c.get("scenePkid").num() > 0)
                r.set("scene", store.get("scenes", c.get("scenePkid").num()));
            else {
                auto files = Json::array();
                for (auto& f : scanVideos(c.get("videoFolder").str()))
                    files.push(Json::string(f));
                r.set("files", files);
            }
        }
        if (!r.has("profileId"))
            r.set("profileId", c.get("profileId").num());
    }
    if (r.get("sceneId").num() > 0)
        r.set("scene", store.get("scenes", r.get("sceneId").num()));
    if (!r.has("profile") && r.get("profileId").num() <= 0) {
        // Un canale senza profilo esplicito usa la scelta attiva; i seed restano template.
        double activeId = 1;
        auto profiles = store.list("profiles");
        for (size_t i = 0; i < profiles.size(); ++i)
            if (profiles.at(i).get("isActive").flag()) {
                activeId = profiles.at(i).get("id").num();
                break;
            }
        r.set("profileId", activeId);
    }
    if (!r.has("profile"))
        r.set("profile", store.get("profiles", r.get("profileId").num(1)));
    validate("profiles", r.get("profile"));
    if (channel && asksForStreamKey(store.get("channels", channel).get("platform").str(),
                                    store.get("channels", channel).get("platformStreamName").str(),
                                    store.get("channels", channel).get("streamUrl").str())) {
        auto p = r.get("profile");
        double w = p.get("videoWidth").num(), h = p.get("videoHeight").num();
        if (w > h) {
            p.set("videoWidth", h);
            p.set("videoHeight", w);
        }
    }
    if (!r.has("outputFile") &&
        (r.get("streamKey").str().empty() || isStandInStreamKey(r.get("streamKey").str())))
        throw std::runtime_error("Chiave richiesta per questa diretta");
    if (r.has("scene")) {
        buildProducer(r.get("scene"), r.get("profile"), 0, {});
    } else {
        if (!r.get("files").size())
            throw std::runtime_error("Playlist vuota");
        for (size_t i = 0; i < r.get("files").size(); ++i)
            probe(r.get("files").at(i).str());
    }
    Json h;
    h.set("status", "STARTING");
    h.set("startedAt", now());
    h.set("channelId", double(channel));
    auto saved = r.clone();
    saved.set("streamKey", "");
    h.set("request", saved);
    auto id = store.save("history", h);
    auto s = std::make_shared<Session>(store, r, id);
    sessions[id] = s;
    s->start();
    return id;
}

// Cerca sotto lock e restituisce un riferimento condiviso, mantenendo viva la sessione durante la
// richiesta.
std::shared_ptr<Session> Studio::session(long id) {
    std::lock_guard lock(mutex);
    auto found = sessions.find(id);
    if (found == sessions.end())
        throw std::runtime_error("Diretta non trovata");
    return found->second;
}

// Aggrega gli snapshot delle sessioni per aggiornare la regia con una singola richiesta.
Json Studio::live() {
    std::lock_guard lock(mutex);
    auto a = Json::array();
    for (auto& [id, s] : sessions)
        a.push(s->snapshot());
    return a;
}

// Ferma le sessioni attive per chiudere i processi esterni quando termina l’applicazione.
void Studio::stopAll() {
    std::vector<std::shared_ptr<Session>> copy;
    {
        std::lock_guard lock(mutex);
        for (auto& [id, s] : sessions)
            copy.push_back(s);
    }
    for (auto& s : copy)
        s->stop();
}
} // namespace orbis
