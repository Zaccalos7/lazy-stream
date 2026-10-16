#include "async_logger.hpp"
#include "core.hpp"
#include "encoder_options.hpp"
#include "history_cleanup_service.hpp"
#include "live_stream_keys.hpp"
#include "overlay_media.hpp"
#include "paced_output.hpp"
#include "platform_lookup.hpp"
#include "rate_control.hpp"
#include "screen_capture.hpp"
#include "stream_platform.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <locale>
#include <poll.h>
#include <regex>
#include <signal.h>
#include <sstream>
#include <unistd.h>

namespace orbis {
namespace {
// Serializza numeri per i filtri FFmpeg senza dipendere dalla virgola decimale della lingua di
// sistema.
std::string number(double n) {
    std::ostringstream s;
    s.imbue(std::locale::classic());
    s << n;
    return s.str();
}

// Accoda argomenti distinti, mantenendo i parametri FFmpeg separati senza quoting da shell.
void add(std::vector<std::string>& a, std::initializer_list<std::string> b) {
    a.insert(a.end(), b);
}

// Windows quality levels describe encoder work, independently of its software/hardware presets.
std::string qualityPreset(const std::string& codec, const std::string& quality) {
    if (codec.ends_with("_nvenc"))
        return quality == "light" ? "p1" : quality == "high" ? "p6" : "p4";
    if (codec.ends_with("_qsv"))
        return quality == "light" ? "veryfast" : quality == "high" ? "slower" : "medium";
    return quality == "light" ? "superfast" : quality == "high" ? "faster" : "veryfast";
}

std::string resolvedPreset(const Json& profile, int width, int height, double fps) {
    const auto codec = videoEncoder(profile);
    auto quality = profile.get("encoderQuality").str();
    if (quality.empty())
        return profile.get("preset").str(codec == "libx264" ? "veryfast" : "medium");
    if (quality == "auto") {
        static std::mutex cacheMutex;
        static std::map<std::string, std::string> measured;
        const auto key = codec + "|" + number(width) + "x" + number(height) + "|" + number(fps);
        std::lock_guard lock(cacheMutex);
        auto found = measured.find(key);
        if (found != measured.end())
            return found->second;
        quality = "light";
        for (const auto& level : {"balanced", "light"}) {
            auto started = std::chrono::steady_clock::now();
            try {
                capture({ffmpeg(),
                         "-v",
                         "error",
                         "-f",
                         "lavfi",
                         "-i",
                         "testsrc2=s=" + number(width) + "x" + number(height) + ":r=" + number(fps),
                         "-t",
                         "3",
                         "-an",
                         "-c:v",
                         codec,
                         "-preset",
                         qualityPreset(codec, level),
                         "-b:v",
                         number(profile.get("videoBitrate").num(4500)) + "k",
                         "-pix_fmt",
                         "yuv420p",
                         "-f",
                         "null",
                         "-"},
                        8);
                const double elapsed =
                    std::chrono::duration<double>(std::chrono::steady_clock::now() - started)
                        .count();
                if (elapsed > 0 && 3 / elapsed >= 2) {
                    quality = level;
                    break;
                }
            } catch (...) {
                // The real producer retains its full failure diagnostic if this encoder cannot
                // open.
            }
        }
        measured[key] = qualityPreset(codec, quality);
        return measured[key];
    }
    return qualityPreset(codec, quality);
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
    auto metadata = probe(file);
    s.set("sourceInfo", metadata);
    auto streams = metadata.get("streams");
    for (size_t i = 0; i < streams.size(); ++i)
        if (streams.at(i).get("codec_type").str() == "video") {
            s.set("width", streams.at(i).get("width"));
            s.set("height", streams.at(i).get("height"));
            auto rate = streams.at(i).get("avg_frame_rate").str();
            auto slash = rate.find('/');
            if (!rate.empty()) {
                double denominator =
                    slash == std::string::npos ? 1 : std::atof(rate.c_str() + slash + 1);
                if (denominator > 0)
                    s.set("frameRate", std::atof(rate.c_str()) / denominator);
            }
            break;
        }
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

std::string streamDiagnostic(const fs::path& path, const Json& request,
                             std::streamoff limit = 4096) {
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return {};
    file.seekg(0, std::ios::end);
    const std::streamoff length = file.tellg();
    file.seekg(length > limit ? length - limit : std::streamoff(0));
    std::string message{std::istreambuf_iterator<char>(file), {}};
    return redactStreamText(message, request);
}
} // namespace

// Redact before writing stderr or commands, including Facebook's separate playpath argument.
std::string redactStreamText(std::string message, const Json& request) {
    const auto key = request.get("streamKey").str();
    if (!key.empty()) {
        size_t pos = 0;
        while ((pos = message.find(key, pos)) != std::string::npos) {
            message.replace(pos, key.size(), "[chiave nascosta]");
            pos += std::string("[chiave nascosta]").size();
        }
    }
    return std::regex_replace(message, std::regex("rtmps?://[^\\s\"']+", std::regex::icase),
                              "[server RTMP]");
}

// Bound each section so a long live can be inspected without loading the entire log into the UI.
std::string readStreamLogs(const fs::path& root, long id) {
    std::string result;
    for (auto suffix : {".session.log", ".publisher.log", ".log", ".preview.log"}) {
        const auto path = root / "logs" / (std::to_string(id) + suffix);
        auto section = streamDiagnostic(path, Json{}, 65536);
        if (!section.empty())
            result += "=== " + path.filename().string() + " ===\n" + section + "\n";
    }
    return result;
}

// Mirrors Windows BuildSender: complete AVC/AAC headers and an explicit Facebook app/playpath.
std::vector<std::string> buildPublisher(const std::string& output) {
    std::vector<std::string> args = {ffmpeg(),
                                     "-hide_banner",
                                     "-nostdin",
                                     "-loglevel",
                                     "level+info",
                                     "-nostats",
                                     "-y",
                                     "-progress",
                                     "pipe:1",
                                     "-stats_period",
                                     "0.2",
                                     "-probesize",
                                     "4194304",
                                     "-analyzeduration",
                                     "2000000",
                                     "-f",
                                     "flv",
                                     "-i",
                                     "pipe:0",
                                     "-map",
                                     "0:v:0",
                                     "-map",
                                     "0:a:0",
                                     "-c",
                                     "copy",
                                     "-flvflags",
                                     "no_duration_filesize",
                                     "-rw_timeout",
                                     "15000000",
                                     "-flush_packets",
                                     "1",
                                     "-f",
                                     "flv"};
    const auto scheme = output.find("://");
    const auto path =
        scheme == std::string::npos ? std::string::npos : output.find('/', scheme + 3);
    if (detectStreamPlatform(output) == StreamPlatform::Facebook && path != std::string::npos &&
        output.compare(path, 6, "/rtmp/") == 0)
        add(args, {"-rtmp_app", "rtmp", "-rtmp_playpath", output.substr(path + 6), "-rtmp_tcurl",
                   output.substr(0, path) + "/rtmp"});
    args.push_back(output);
    return args;
}

// Costruisce ingressi e filtri FFmpeg per video, audio e anteprima; una pipeline comune mantiene
// coerente l’uscita.
Build buildProducer(const Json& scene, const Json& profile, double offset,
                    const fs::path& preview) {
    Build result;
    const bool previewOnly = profile.get("previewOnly").flag();
    const auto codec = previewOnly ? std::string("mjpeg") : videoEncoder(profile);
    const bool vaapi = codec.ends_with("_vaapi");
    const auto pixelFormat = previewOnly ? std::string("yuv420p")
                             : vaapi     ? "nv12"
                                         : videoPixelFormat(profile);
    auto& a = result.args;
    a = {ffmpeg(),
         "-hide_banner",
         "-nostdin",
         "-loglevel",
         "level+info",
         "-nostats",
         "-filter_complex_threads",
         "1",
         "-threads",
         "2",
         "-y"};
    if (vaapi) {
        const auto device = profile.get("hardwareDevice").str("/dev/dri/renderD128");
        if (!device.starts_with("/dev/dri/"))
            throw std::runtime_error("Dispositivo VAAPI non valido");
        add(a, {"-vaapi_device", device});
    }
    int w = profile.get("videoWidth").num(scene.get("width").num(1280)),
        h = profile.get("videoHeight").num(scene.get("height").num(720));
    double fps = profile.get("frameRate").num(scene.get("frameRate").num(30));
    if (w < 16 || h < 16 || w > 7680 || h > 7680 || w % 2 || h % 2 || fps < 1 || fps > 240)
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
    std::vector<bool> picture, stillPicture;
    int input = 0;
    bool finite = false, capturePresent = false;
    double duration = 0;
    for (size_t i = 0; i < items.size(); ++i) {
        const auto k = kind(items.at(i));
        capturePresent |= k == "SCREEN" || k == "CAMERA" || k == "MICROPHONE";
    }
    for (size_t i = 0; i < items.size(); ++i) {
        auto t = items.at(i);
        auto target = t.get("sourceTarget").str();
        auto k = kind(t);
        if (target.empty()) {
            picture.push_back(false);
            stillPicture.push_back(false);
            continue;
        }
        bool au = false, vi = k != "MICROPHONE", staticPicture = false;
        if (k == "SCREEN" || k == "CAMERA" || k == "MICROPHONE")
            capturePresent = true;
        if (k == "SCREEN") {
            const auto screen = screenInput(target, fps);
            a.insert(a.end(), screen.begin(), screen.end());
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
            const auto media = OverlayMedia::of(p);
            staticPicture = !media.loop;
            if (media.nativeWebpLoop && (k == "OVERLAY" || t.get("loop").flag())) {
                auto copy = std::make_shared<WebpOverlayInput>(target);
                target = copy->path.string();
                result.resources.push_back(std::move(copy));
                add(a, {"-ignore_loop", "0"});
            } else if (!staticPicture && (k == "OVERLAY" || t.get("loop").flag()))
                add(a, {"-stream_loop", "-1"});
            if (!media.decoder.empty())
                add(a, {"-c:v", media.decoder});
            if (offset > 0 && k == "FILE" && !staticPicture)
                add(a, {"-ss", number(offset)});
            if (!staticPicture && (!profile.get("queueRelay").flag() || capturePresent))
                a.push_back("-re");
            add(a, {"-i", target});
            double d = p.get("format").get("duration").num();
            if (k == "FILE" && !staticPicture && !t.get("loop").flag() && d > 0) {
                duration = std::max(duration, d - offset);
                finite = true;
            }
        } else
            throw std::runtime_error("Tipo sorgente non supportato");
        picture.push_back(vi);
        stillPicture.push_back(staticPicture);
        if (vi) {
            int tw = t.get("width").num(w), th = t.get("height").num(h);
            if (tw < 1 || th < 1 || tw > 7680 || th > 7680)
                throw std::runtime_error("Dimensioni tile non valide");
            graph += "[" + std::to_string(input) + ":v]setpts=PTS-STARTPTS," +
                     std::string(previewOnly ? "fps=2," : "") + "scale=" + std::to_string(tw) +
                     ":" + std::to_string(th) +
                     ":force_original_aspect_ratio=decrease,pad=" + std::to_string(tw) + ":" +
                     std::to_string(th) +
                     ":(ow-iw)/2:(oh-ih)/2:color=black@0,setsar=1,format=rgba[v" +
                     std::to_string(i) + "];";
        }
        if (au && k != "OVERLAY" && t.get("audioEnabled").flag(true)) {
            audio.push_back(input);
            volumes.push_back(std::clamp(t.get("volume").num(100), 0.0, 200.0) / 100);
        }
        ++input;
    }
    if (previewOnly)
        a.push_back("-re");
    add(a, {"-f", "lavfi", "-i",
            "color=c=black:s=" + std::to_string(w) + "x" + std::to_string(h) +
                ":r=" + number(previewOnly ? 2 : fps)});
    int background = input++;
    graph += "[" + std::to_string(background) + ":v]setpts=PTS-STARTPTS[base0];";
    int layer = 0;
    for (size_t i = 0; i < items.size(); ++i)
        if (picture[i]) {
            auto t = items.at(i);
            graph += "[base" + std::to_string(layer) + "][v" + std::to_string(i) +
                     "]overlay=x=" + number(t.get("x").num()) + ":y=" + number(t.get("y").num()) +
                     (stillPicture[i] ? ":eof_action=repeat:shortest=0[base"
                                      : ":eof_action=pass:shortest=0[base") +
                     std::to_string(layer + 1) + "];";
            ++layer;
        }
    graph += "[base" + std::to_string(layer) + "]fps=" + number(previewOnly ? 2 : fps) +
             ",format=" + pixelFormat;
    if (!preview.empty() && !previewOnly) {
        graph +=
            ",split=2[outv][preview];[preview]fps=" + number(std::min(30., fps)) +
            (h > w ? ",scale=w=-2:h='min(640,ih)'[thumb];" : ",scale=w='min(640,iw)':h=-2[thumb];");
    } else
        graph += "[outv];";
    if (audio.empty()) {
        add(a, {"-f", "lavfi", "-i",
                "anullsrc=r=" + number(profile.get("audioSampleRate").num(48000)) + ":cl=stereo"});
        graph += "[" + std::to_string(input) + ":a]anull[outa]";
    } else {
        for (size_t i = 0; i < audio.size(); ++i)
            graph += "[" + std::to_string(audio[i]) + ":a]asetpts=PTS-STARTPTS,aresample=" +
                     number(profile.get("audioSampleRate").num(48000)) +
                     ",aformat=channel_layouts=stereo,volume=" + number(volumes[i]) + "[a" +
                     std::to_string(i) + "];";
        for (size_t i = 0; i < audio.size(); ++i)
            graph += "[a" + std::to_string(i) + "]";
        graph +=
            "amix=inputs=" + std::to_string(audio.size()) + ":duration=longest:normalize=0[outa]";
    }
    if (previewOnly) {
        graph += ";[outa]anullsink";
        add(a, {"-filter_complex",
                graph,
                "-map",
                "[outv]",
                "-an",
                "-r",
                "2",
                "-c:v",
                "mjpeg",
                "-q:v",
                "5",
                "-threads",
                "1",
                "-f",
                "image2",
                "-update",
                "1",
                "-atomic_writing",
                "1",
                preview.string()});
        return result;
    }
    if (vaapi)
        graph += ";[outv]format=nv12,hwupload[encoded]";
    add(a, {"-filter_complex", graph, "-map", vaapi ? "[encoded]" : "[outv]", "-map", "[outa]"});
    auto bitrate = number(profile.get("videoBitrate").num(4500)) + "k";
    add(a,
        {"-c:v", codec, "-b:v", bitrate, "-maxrate", bitrate, "-bufsize",
         number(profile.get("videoBitrate").num(4500) * profile.get("rateBufferSeconds").num(2)) +
             "k",
         "-g", number(std::round(profile.get("gopSize").num(2) * fps)), "-pix_fmt",
         vaapi ? "vaapi" : pixelFormat, "-force_key_frames",
         "expr:gte(t,n_forced*" + number(profile.get("gopSize").num(2)) + ")"});
    if (codec == "libx264" || codec == "libx265" || codec.ends_with("_nvenc") ||
        codec.ends_with("_qsv"))
        add(a, {"-preset", resolvedPreset(profile, w, h, fps)});
    if (codec == "libx264") {
        if (profileSwitch(profile, "lowLatency", "lowlatency"))
            add(a, {"-tune", "zerolatency"});
        else if (!profile.get("tune").str().empty())
            add(a, {"-tune", profile.get("tune").str()});
        if (profile.get("constantBitrate").flag())
            add(a, {"-minrate", bitrate});
    }
    const auto options = encoderOptions(profile, codec);
    for (size_t i = 0; i < options.size(); ++i) {
        const auto o = options.at(i);
        auto key = o.get("key").str();
        if (key == "profile")
            key = "profile:v";
        if (key == "lowlatency" || key == "adaptivebitrate" || key == "encoderquality" ||
            key == "x264-params")
            continue;
        if (key == "crf" || key == "profile:v" || key == "level" || key == "bf" || key == "refs" ||
            key == "preset" || key == "tune" || key == "rc" || key == "multipass" ||
            key == "rc-lookahead" || key == "zerolatency" || key == "b_ref_mode" ||
            key == "spatial-aq" || key == "temporal-aq" || key == "threads" ||
            encoderOptionSupported(codec, key))
            add(a, {"-" + key, o.get("value").str()});
        else
            throw std::runtime_error("Opzione FFmpeg non consentita: " + key);
    }
    if (codec == "libx264") {
        const auto parameters = x264Parameters(options, profile.get("constantBitrate").flag());
        if (!parameters.empty())
            add(a, {"-x264-params", parameters});
    }
    add(a,
        {"-c:a", audioEncoder(profile), "-b:a", number(profile.get("audioBitrate").num(128)) + "k",
         "-ar", number(profile.get("audioSampleRate").num(48000)), "-ac",
         number(profile.get("audioChannels").num(2))});
    if (capturePresent)
        finite = false;
    if (finite) {
        result.duration = std::max(0.05, duration);
        add(a, {"-t", number(result.duration)});
    }
    add(a, {"-flvflags", "no_duration_filesize", "-flush_packets", "1", "-f", "flv", "pipe:1"});
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
    cursor = 0;
    header = false;
    firstTag = true;
    first = lastRaw = wrap = lastVideo = 0;
    base = sentHeader ? last + frameStep : 0;
    if (!sentHeader)
        epoch = std::chrono::steady_clock::now();
}

// Reassemble once, move whole compressed tags to the sender, and compact only when necessary.
std::vector<FlvChunk> FlvRelay::packets(const unsigned char* bytes, size_t size) {
    if (cursor && (cursor >= 65536 || cursor == pending.size())) {
        pending.erase(pending.begin(), pending.begin() + cursor);
        cursor = 0;
    }
    pending.insert(pending.end(), bytes, bytes + size);
    if (pending.size() - cursor > 32 * 1024 * 1024)
        throw std::runtime_error("FLV oltre il limite");
    std::vector<FlvChunk> out;
    if (!header) {
        if (pending.size() - cursor < 13)
            return out;
        const auto* p = pending.data() + cursor;
        if (p[0] != 'F' || p[1] != 'L' || p[2] != 'V' || p[3] != 1)
            throw std::runtime_error("Header FLV non valido");
        const uint32_t offset =
            (uint32_t(p[5]) << 24) | (uint32_t(p[6]) << 16) | (uint32_t(p[7]) << 8) | p[8];
        if (offset < 9 || offset > 1024 * 1024)
            throw std::runtime_error("Offset FLV non valido");
        if (pending.size() - cursor < offset + 4)
            return out;
        if (!sentHeader) {
            out.push_back({{'F', 'L', 'V', 1, p[4], 0, 0, 0, 9, 0, 0, 0, 0}, 0, false, {}});
            sentHeader = true;
        }
        cursor += offset + 4;
        header = true;
    }
    while (pending.size() - cursor >= 15) {
        const auto* p = pending.data() + cursor;
        const uint32_t len = u24(p + 1);
        if (pending.size() - cursor < len + 15)
            break;
        const uint32_t previous = (uint32_t(p[11 + len]) << 24) | (uint32_t(p[12 + len]) << 16) |
                                  (uint32_t(p[13 + len]) << 8) | p[14 + len];
        if (previous != len + 11 || (p[0] != 8 && p[0] != 9 && p[0] != 18) || u24(p + 8))
            throw std::runtime_error("Tag FLV non valido");
        if (p[0] == 18) {
            // A publish has one metadata header. Repeating it at producer changes creates a
            // spurious data stream in demuxers and describes an encoder which is already gone.
            if (metadataSent) {
                cursor += len + 15;
                continue;
            }
            metadataSent = true;
        }
        if (p[0] == 8 && len > 2 && (p[11] >> 4) == 10 && p[12] == 0) {
            // FFmpeg 9 can reapply the legacy FLV 44.1 kHz flags after an identical repeated AAC
            // configuration. Keep changed configurations, send an unchanged ASC just once.
            if (audioConfiguration.size() == len &&
                std::equal(audioConfiguration.begin(), audioConfiguration.end(), p + 11)) {
                cursor += len + 15;
                continue;
            }
            audioConfiguration.assign(p + 11, p + 11 + len);
        }
        if (p[0] == 9 && len >= 2 && (p[11] & 15) == 7 && p[12] == 2) {
            cursor += len + 15;
            continue; // A producer ended; the shared publish has not.
        }
        const auto raw = uint64_t(u24(p + 4) | (uint32_t(p[7]) << 24));
        const bool sequence =
            len >= 2 && ((p[0] == 9 && (p[11] & 128) && (p[11] & 15) != 1 && (p[11] & 15) != 3) ||
                         (p[0] == 8 && (p[11] >> 4) == 9 && (p[11] & 15) != 1) ||
                         (p[0] == 9 && (p[11] & 15) == 7 && p[12] != 1) ||
                         (p[0] == 8 && (p[11] >> 4) == 10 && p[12] == 0));
        const bool media = (p[0] == 8 || p[0] == 9) && !sequence;
        if (media && raw < lastRaw && lastRaw - raw > 0x80000000ULL)
            wrap += 1ULL << 32;
        const auto timestamp = raw + wrap;
        if (media && firstTag) {
            first = timestamp;
            firstTag = false;
        }
        uint64_t normalized =
            firstTag || !media ? base : base + (timestamp >= first ? timestamp - first : 0);
        if (media && normalized > last + 5000 && last > base) {
            // Discontinuities are rebased; network lateness never rewrites this clock.
            first += normalized - last - frameStep;
            normalized = last + frameStep;
        }
        normalized = std::max(last, normalized);
        if (media) {
            if (p[0] == 9) {
                if (lastVideo && normalized > lastVideo && normalized - lastVideo < 1000)
                    frameStep = normalized - lastVideo;
                lastVideo = normalized;
            }
            last = normalized;
            lastRaw = raw;
        }
        FlvChunk chunk{{p, p + len + 15}, normalized, media, {}};
        put24(chunk.bytes.data() + 4, uint32_t(normalized));
        chunk.bytes[7] = normalized >> 24;
        out.push_back(std::move(chunk));
        cursor += len + 15;
    }
    return out;
}

// Compatibility helper for callers/tests that need one FLV byte vector rather than queued tags.
std::vector<unsigned char> FlvRelay::feed(const unsigned char* bytes, size_t size, bool pace) {
    std::vector<unsigned char> out;
    for (auto& chunk : packets(bytes, size))
        out.insert(out.end(), chunk.bytes.begin(), chunk.bytes.end());
    if (pace && last) {
        const auto target = epoch + std::chrono::milliseconds(last);
        std::this_thread::sleep_until(target);
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
        auto items = original.get("items");
        for (size_t i = 0; i < items.size(); ++i)
            if (kind(items.at(i)) == "FILE" && !items.at(i).get("sourceTarget").str().empty()) {
                original.set("sourceInfo", probe(items.at(i).get("sourceTarget").str()));
                break;
            }
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
        index = int(r.get("startIndex").num());
        original = playlist[index];
    }
    offset = r.get("offsetSeconds").num();
    videoResults = Json::array();
    for (size_t i = 0; i < playlist.size(); ++i) {
        Json result;
        result.set("status", i < size_t(index) ? "ENDED" : "OFFLINE");
        result.set("error", "");
        videoResults.push(result);
    }
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
void Session::stop(const std::string& reason) {
    std::lock_guard lock(stopMutex);
    if (!reason.empty()) {
        std::lock_guard stateLock(mutex);
        stopReason = reason;
    }
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
    s.set("channelName", request.get("channelName"));
    s.set("platform", request.get("platform"));
    const auto platform =
        streamPlatform(request.get("streamUrl").str(), request.get("platform").str());
    const char* playerPlatform = platform == StreamPlatform::Twitch     ? "Twitch"
                                 : platform == StreamPlatform::YouTube  ? "YouTube"
                                 : platform == StreamPlatform::Kick     ? "Kick"
                                 : platform == StreamPlatform::Facebook ? "Facebook"
                                 : platform == StreamPlatform::TikTok   ? "TikTok"
                                                                        : "Custom";
    s.set("playerPlatform", playerPlatform);
    s.set("startedAt", request.get("startedAt"));
    s.set("status", state);
    s.set("error", error);
    s.set("position", double(index));
    s.set("total", double(playlist.size()));
    s.set("offsetSeconds", offset);
    s.set("scene", original);
    s.set("takeover", takeover);
    const auto onAir = delivery.get("context");
    if (onAir.isObject()) {
        s.set("position", onAir.get("position"));
        s.set("scene", onAir.get("scene"));
        s.set("takeover", onAir.get("takeover"));
        const double advance =
            onAir.get("advances").flag()
                ? std::max(0., delivery.get("timestamp").num() - onAir.get("base").num()) / 1000
                : 0;
        s.set("offsetSeconds", onAir.get("offset").num() + advance);
    }
    s.set("producerPid", double(producerPid));
    s.set("publisherPid", double(publisherPid));
    if (producerPid > 0)
        s.set("producerMetrics", processMetrics(producerPid));
    if (publisherPid > 0)
        s.set("publisherMetrics", processMetrics(publisherPid));
    s.set("profile", profile);
    s.set("output", outputProfile);
    if (onAir.isObject())
        s.set("output", onAir.get("profile"));
    s.set("videoResults", videoResults);
    s.set("delivery", delivery);
    s.set("outputKbps", outputKbps);
    s.set("reconnectAttempts", double(reconnectAttempts));
    // JSON nodes use shared, non-atomic json-c references. Detach the whole snapshot while the
    // session lock is held: UI/API readers must not share nodes being mutated by the worker.
    return s.clone();
}

// Aggiorna i comandi sotto mutex e segnala il cambio al worker, evitando interventi concorrenti sui
// processi.
void Session::command(const std::string& name, const Json& data) {
    std::lock_guard lock(mutex);
    if (finished)
        throw std::runtime_error("Diretta terminata");
    const auto onAir = delivery.get("context");
    if (onAir.isObject() && name != "spot") {
        const int current = onAir.get("position").num();
        if (current >= 0 && current < int(playlist.size())) {
            index = current;
            original = playlist[index];
            offset =
                onAir.get("offset").num() +
                (onAir.get("advances").flag()
                     ? std::max(0., delivery.get("timestamp").num() - onAir.get("base").num()) /
                           1000
                     : 0);
        }
    }
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
    store.changes.raise();
}

// Mantiene il publisher aperto mentre sostituisce i produttori; gestisce backpressure, watchdog e
// tentativi di riconnessione.
void Session::run() {
    int relayPipe[2]{-1, -1};
    Process publisher, producer;
    FlvRelay relay;
    auto recordVideo = [&](const std::string& status, const std::string& message = "",
                           int selected = -1) {
        Json results;
        {
            std::lock_guard lock(mutex);
            const int videoIndex = selected >= 0 ? selected : index;
            if (videoIndex >= 0 && videoIndex < int(videoResults.size())) {
                auto video = videoResults.at(videoIndex);
                video.set("status", status);
                video.set("error", message);
                video.set("offsetSeconds", offset);
            }
            results = videoResults.clone();
        }
        auto history = store.get("history", id);
        history.set("videoResults", results);
        store.save("history", history, id);
    };
    const auto publisherLog = store.root / "logs" / (std::to_string(id) + ".publisher.log");
    const auto producerLog = store.root / "logs" / (std::to_string(id) + ".log");
    const auto sessionLog = store.root / "logs" / (std::to_string(id) + ".session.log");
    auto redact = [this](const std::string& message) { return redactStreamText(message, request); };
    auto log = [&](const std::string& phase, const std::string& message) {
        store.logger->write(phase, "live=" + std::to_string(id) + " " + redact(message),
                            sessionLog);
    };
    auto logCommand = [&](const std::string& role, const std::vector<std::string>& args) {
        auto command = Json::array();
        for (const auto& arg : args)
            command.push(Json::string(redact(arg)));
        log(role, command.dump());
    };
    std::atomic<bool> publisherCancelled{false};
    std::unique_ptr<NativePublisher> native;
    std::unique_ptr<PacedOutput> paced;
    int reconnectLimit = 3;
    std::string phase = "prepare";
    log(phase, "attempt=" + std::to_string(reconnectAttempts + 1) +
                   " channel=" + request.get("channelName").str());
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
        const auto platform = streamPlatform(output, request.get("platform").str());
        const auto platformProfile = deliveryFor(platform);
        reconnectLimit = platformProfile.reconnectAttempts;
        const bool buffered = platformProfile.buffered;
        const bool senderClock =
            request.get("transport").str() != "native" && buffered &&
            !(platform == StreamPlatform::YouTube && !request.get("ffmpegSender").flag(true));
        const bool nativeMode = NativePublisher::available() && !senderClock &&
                                request.get("transport").str() != "ffmpeg";
        if (request.get("transport").str() == "native" && !NativePublisher::available())
            throw std::runtime_error("Native transport requires FFmpeg 9.0.2 libraries");
        const auto cancelled = [&] { return stopped.load() || publisherCancelled.load(); };
        const int64_t maxLead = platformProfile.maxLeadMilliseconds;
        phase = "publisher-start";
        if (nativeMode) {
            native =
                std::make_unique<NativePublisher>(output, cancelled, [&](const std::string& line) {
                    store.logger->write("publisher:" + std::to_string(id), redact(line),
                                        publisherLog);
                });
            log(phase, "native libavformat transport; no sender process");
        } else {
            if (pipe2(relayPipe, O_CLOEXEC))
                throw std::runtime_error("Relay pipe fallita");
            auto publisherArgs = buildPublisher(output);
            if (senderClock) {
                const auto input = std::find(publisherArgs.begin(), publisherArgs.end(), "-f");
                publisherArgs.insert(input, {"-readrate", "1", "-readrate_initial_burst", "2"});
            }
            logCommand(phase, publisherArgs);
            publisher.start(publisherArgs, relayPipe[0], publisherLog, true, redact);
            log(phase, "pid=" + std::to_string(publisher.id()));
            close(relayPipe[0]);
            relayPipe[0] = -1;
            fcntl(relayPipe[1], F_SETFL, O_NONBLOCK);
        }
        paced = std::make_unique<PacedOutput>(
            maxLead, platformProfile.prerollMilliseconds, !senderClock, cancelled,
            [&] {
                if (native)
                    native->open();
            },
            [&](const FlvChunk& chunk) {
                if (native) {
                    native->write(chunk);
                    return;
                }
                size_t sent = 0;
                auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
                while (sent < chunk.bytes.size() && !cancelled()) {
                    pollfd fd{relayPipe[1], POLLOUT, 0};
                    const auto ready = poll(&fd, 1, 50);
                    if (ready < 0 && errno == EINTR)
                        continue;
                    if (ready < 0 || (fd.revents & (POLLERR | POLLHUP)))
                        throw std::runtime_error("Publisher pipe disconnected");
                    if (ready > 0) {
                        const auto count = write(relayPipe[1], chunk.bytes.data() + sent,
                                                 chunk.bytes.size() - sent);
                        if (count > 0)
                            sent += size_t(count);
                        else if (errno != EAGAIN && errno != EINTR)
                            throw std::runtime_error("Publisher pipe write failed");
                    }
                    if (std::chrono::steady_clock::now() > deadline)
                        throw std::runtime_error("Publisher pipe stalled for 30 seconds");
                }
            },
            [&] {
                if (native)
                    native->close();
            });
        {
            std::lock_guard lock(mutex);
            publisherPid = nativeMode ? -1 : publisher.id();
            state = reconnectAttempts ? "RECONNECTING" : "STARTING";
            error = stopReason;
            outputKbps = 0;
        }
        bool publishing = false;
        int onAirVideo = -1;
        std::string progress;
        const auto connectStarted = std::chrono::steady_clock::now();
        auto lastProgress = connectStarted;
        long long lastOutputTime = 0;
        auto rateSample = paced->sample();
        auto checkPublisher = [&] {
            paced->check();
            auto current = paced->sample();
            if (nativeMode) {
                const auto counters = paced->snapshot();
                if (counters.get("published").flag() &&
                    (current.media > lastOutputTime || !publishing)) {
                    lastOutputTime = current.media;
                    lastProgress = std::chrono::steady_clock::now();
                }
            } else {
                pollfd fd{publisher.fd(), POLLIN, 0};
                if (poll(&fd, 1, 0) > 0 && (fd.revents & POLLIN)) {
                    char buffer[4096];
                    const auto count = read(publisher.fd(), buffer, sizeof(buffer));
                    if (count > 0)
                        progress.append(buffer, count);
                    size_t newline;
                    while ((newline = progress.find('\n')) != std::string::npos) {
                        const auto line = progress.substr(0, newline);
                        progress.erase(0, newline + 1);
                        if (line.starts_with("out_time_us=")) {
                            const auto stamp = std::strtoll(line.c_str() + 12, nullptr, 10) / 1000;
                            if (stamp > lastOutputTime) {
                                lastOutputTime = stamp;
                                lastProgress = std::chrono::steady_clock::now();
                            }
                        }
                    }
                }
                if (!publisher.running()) {
                    phase = "publisher-exit";
                    throw std::runtime_error("Publisher terminato, exit=" +
                                             std::to_string(publisher.wait()));
                }
            }
            if (!publishing &&
                (lastOutputTime > 0 || (nativeMode && paced->snapshot().get("published").flag()))) {
                publishing = true;
                log("publishing", "first delivered timestamp=" + std::to_string(lastOutputTime));
                {
                    std::lock_guard lock(mutex);
                    state = "LIVE";
                }
                recordVideo("LIVE");
                auto history = store.get("history", id);
                history.set("status", "LIVE");
                store.save("history", history, id);
            }
            auto counters = paced->snapshot();
            if (publishing && counters.has("context")) {
                const int video = counters.get("context").get("position").num();
                if (video != onAirVideo) {
                    if (onAirVideo >= 0)
                        recordVideo("ENDED", "", onAirVideo);
                    recordVideo("LIVE", "", video);
                    onAirVideo = video;
                }
            }
            counters.set("transport", nativeMode ? "native-libavformat" : "ffmpeg-sender");
            {
                std::lock_guard lock(mutex);
                delivery = counters;
                if (current.ticks - rateSample.ticks >= 1000) {
                    outputKbps = publishing ? (current.bytes - rateSample.bytes) * 8. /
                                                  (current.ticks - rateSample.ticks)
                                            : 0;
                    rateSample = current;
                }
            }
            const auto now = std::chrono::steady_clock::now();
            if (!publishing &&
                now - connectStarted > std::chrono::seconds(platformProfile.connectSeconds))
                throw std::runtime_error("Timeout avvio publisher: nessun avanzamento in " +
                                         std::to_string(platformProfile.connectSeconds) +
                                         " secondi");
            if (publishing &&
                now - lastProgress > std::chrono::seconds(platformProfile.stallSeconds))
                throw std::runtime_error("Publisher bloccato: nessun avanzamento in " +
                                         std::to_string(platformProfile.stallSeconds) + " secondi");
        };
        RateControl rateControl(buffered, maxLead);
        auto h = store.get("history", id);
        h.set("status", reconnectAttempts ? "RECONNECTING" : "STARTING");
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
            phase = "producer-build";
            params.set("queueRelay", true);
            if (platformProfile.uniformFormat) {
                if (outputProfile.has("videoWidth")) {
                    params.set("videoWidth", outputProfile.get("videoWidth"));
                    params.set("videoHeight", outputProfile.get("videoHeight"));
                    params.set("frameRate", outputProfile.get("frameRate"));
                } else {
                    const auto frame = frameOfLive(
                        {int(params.get("videoWidth").num(scene.get("width").num(1280))),
                         int(params.get("videoHeight").num(scene.get("height").num(720))),
                         params.get("frameRate").num(scene.get("frameRate").num(30))},
                        params.get("videoWidth").num() > 0 && params.get("videoHeight").num() > 0);
                    params.set("videoWidth", double(frame.width));
                    params.set("videoHeight", double(frame.height));
                    params.set("frameRate", frame.frameRate);
                }
            }
            if (!isTakeover)
                recordVideo(publishing ? "LIVE" : "STARTING");
            params.set("audioSampleRate", double(platformProfile.audioSampleRate));
            if (platformProfile.uniformFormat)
                params.set("audioChannels", 2.);
            params.set("constantBitrate", platformProfile.constantBitrate);
            params.set("rateBufferSeconds", platform == StreamPlatform::Generic ? 2. : 1.);
            if (platformProfile.portrait) {
                const auto w = params.get("videoWidth").num(scene.get("width").num(1280));
                const auto h = params.get("videoHeight").num(scene.get("height").num(720));
                params.set("videoWidth", std::min(w, h));
                params.set("videoHeight", std::max(w, h));
            }
            log(phase, "segment=" + std::to_string(segmentIndex) +
                           " offset=" + number(startOffset) +
                           " sources=" + std::to_string(scene.get("items").size()));
            auto build = buildProducer(scene, params, startOffset, frame);
            {
                std::lock_guard lock(mutex);
                outputProfile = params.clone();
                outputProfile.set("videoWidth",
                                  params.get("videoWidth").num(scene.get("width").num(1280)));
                outputProfile.set("videoHeight",
                                  params.get("videoHeight").num(scene.get("height").num(720)));
                outputProfile.set("frameRate",
                                  params.get("frameRate").num(scene.get("frameRate").num(30)));
            }
            double takeoverDuration = 0;
            if (isTakeover) {
                std::lock_guard lock(mutex);
                takeoverDuration = takeover.get("durationSeconds").num();
            }
            phase = "producer-start";
            logCommand(phase, build.args);
            producer.start(build.args, -1, producerLog, true, redact);
            log(phase, "pid=" + std::to_string(producer.id()));
            phase = "relay";
            {
                std::lock_guard lock(mutex);
                producerPid = producer.id();
            }
            relay.segment();
            auto mediaStart = relay.segmentBase();
            Json context;
            {
                std::lock_guard lock(mutex);
                context.set("position", double(segmentIndex));
                context.set("offset", offset);
                context.set("scene", original.clone());
                context.set("takeover", takeover.clone());
                context.set("profile", outputProfile.clone());
            }
            context.set("base", double(mediaStart));
            context.set("advances", !isTakeover || inScene);
            auto packetContext = std::make_shared<const Json>(context.clone());
            auto lastByte = std::chrono::steady_clock::now();
            bool received = false;
            // A batch can contain several seconds of simple video plus AAC. Limit each read
            // so backpressure cannot defer a new scene/mixer command behind a whole batch.
            unsigned char bytes[4096];
            while (!stopped && !changed) {
                pollfd rd{producer.fd(), POLLIN, 0};
                int n = poll(&rd, 1, 100);
                if (n < 0 && errno == EINTR)
                    continue;
                if (n < 0)
                    throw std::runtime_error("Lettura producer: poll fallita");
                if (n > 0) {
                    auto count = read(producer.fd(), bytes, sizeof(bytes));
                    if (count <= 0)
                        break;
                    if (!received)
                        log("producer-output", "first FLV bytes=" + std::to_string(count));
                    received = true;
                    lastByte = std::chrono::steady_clock::now();
                    for (auto& packet : relay.packets(bytes, count)) {
                        packet.context = packetContext;
                        paced->push(std::move(packet));
                    }
                    checkPublisher();
                    if (!isTakeover || inScene) {
                        std::lock_guard lock(mutex);
                        if (index == segmentIndex)
                            offset = startOffset + std::max(0., double(paced->sample().media) -
                                                                    double(mediaStart)) /
                                                       1000;
                    }
                    if (profileSwitch(params, "adaptiveBitrate", "adaptivebitrate")) {
                        if (auto decision =
                                rateControl.observe(paced->sample(),
                                                    params.get("targetBitrate")
                                                        .num(params.get("videoBitrate").num(4500)),
                                                    params.get("audioBitrate").num(128))) {
                            log("rate-adaptation", "video kbps=" + std::to_string(*decision));
                            std::lock_guard lock(mutex);
                            profile.set("videoBitrate", double(*decision));
                            if (!videoProfiles.empty())
                                videoProfiles[index].set("videoBitrate", double(*decision));
                            changed = true;
                        }
                    }
                }
                if (isTakeover && takeoverDuration > 0 &&
                    relay.timestamp() - mediaStart >= takeoverDuration * 1000) {
                    producer.stop();
                    break;
                }
                if (std::chrono::steady_clock::now() - lastByte > std::chrono::seconds(30))
                    throw std::runtime_error("FFmpeg non produce dati da 30 secondi");
                checkPublisher();
            }
            bool interrupted = changed || stopped;
            int result = interrupted ? (producer.stop(), 0) : producer.wait();
            phase = "producer-exit";
            log("producer-exit",
                "exit=" + std::to_string(result) + " interrupted=" + std::to_string(interrupted));
            if (!interrupted && (!received || result != 0))
                throw std::runtime_error("Codifica fallita, exit=" + std::to_string(result));
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
        paced->finish();
        if (!stopped && nativeMode)
            checkPublisher();
        if (relayPipe[1] >= 0) {
            close(relayPipe[1]);
            relayPipe[1] = -1;
        }
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (publisher.running() && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (!stopped && publisher.running())
            throw std::runtime_error("Timeout chiusura publisher: dati finali non confermati");
        const bool publisherFailed = !stopped && !publisher.running() && publisher.wait() != 0;
        publisher.stop();
        if (publisherFailed)
            throw std::runtime_error("Publisher fallito durante la chiusura, exit=" +
                                     std::to_string(publisher.wait()));
        {
            std::lock_guard lock(mutex);
            // Uno stop richiesto dall'utente è distinto dalla fine naturale della playlist.
            state = stopped ? "STOPPED" : "ENDED";
        }
    } catch (const std::exception& e) {
        publisherCancelled = true;
        if (paced)
            paced->abort();
        if (native)
            native->close();
        producer.stop();
        publisher.stop();
        log("error", "phase=" + phase + " error=" + e.what() +
                         " producerExit=" + std::to_string(producer.wait()) +
                         " publisherExit=" + std::to_string(publisher.wait()));
        store.logger->flush();
        auto detail = "[producer]\n" + streamDiagnostic(producerLog, request) + "\n[publisher]\n" +
                      streamDiagnostic(publisherLog, request);
        std::lock_guard lock(mutex);
        const auto nextState = stopped ? "STOPPED" : "ERROR";
        const auto nextError =
            stopped ? "" : redact("phase=" + phase + ": " + e.what() + "\n" + detail);
        // Publish the diagnostic in history before readers can observe a terminal session status.
        // Per-video finalization below can involve another database write.
        try {
            auto history = store.get("history", id);
            const bool reconnecting = !stopped && request.get("reconnect").flag(true) &&
                                      reconnectAttempts < reconnectLimit;
            history.set("status", reconnecting ? "RECONNECTING" : nextState);
            history.set("error", nextError);
            store.save("history", history, id);
        } catch (const std::exception& persistError) {
            log("history-error", persistError.what());
        }
        state = nextState;
        error = nextError;
    }
    publisherCancelled = true;
    if (paced)
        paced->abort();
    if (native)
        native->close();
    producer.stop();
    publisher.stop();
    for (auto fd : relayPipe)
        if (fd >= 0)
            close(fd);
    bool retry = false;
    {
        std::lock_guard lock(mutex);
        retry = state == "ERROR" && !stopped && request.get("reconnect").flag(true) &&
                reconnectAttempts < reconnectLimit;
        if (retry) {
            ++reconnectAttempts;
            state = "RECONNECTING";
        }
    }
    if (retry) {
        log("reconnect", "next attempt=" + std::to_string(reconnectAttempts + 1));
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
        if (stopped) {
            state = "STOPPED";
            error = stopReason;
        }
    }
    try {
        auto h = store.get("history", id);
        auto s = snapshot();
        if (s.get("status").str() == "ERROR" || s.get("status").str() == "STOPPED" ||
            s.get("status").str() == "ENDED")
            recordVideo(s.get("status").str(), s.get("error").str(), s.get("position").num());
        h.set("videoResults", snapshot().get("videoResults"));
        h.set("status", s.get("status"));
        h.set("error", s.get("error"));
        h.set("offsetSeconds", s.get("offsetSeconds"));
        h.set("position", s.get("position"));
        h.set("endedAt", now());
        store.save("history", h, id);
    } catch (...) {
    }
    log("finished", "status=" + snapshot().get("status").str());
    store.logger->flush();
    finished = true;
}

// Avvia la manutenzione periodica separatamente dalle richieste, per applicare la conservazione di
// storico e log.
Studio::Studio(const fs::path& root) : store(root) {
    historyCleanup = std::make_unique<LiveHistoryCleanupService>(store);
    youtubeAir = std::make_unique<YouTubeAirState>();
    airWatcher = std::jthread([this](std::stop_token stop) {
        std::mutex waitMutex;
        std::condition_variable_any changed;
        std::unique_lock waitLock(waitMutex);
        while (!changed.wait_for(waitLock, stop, std::chrono::seconds(30), [] { return false; })) {
            if (stop.stop_requested())
                break;
            try {
                checkYouTube(stop);
            } catch (...) { /* A lookup must never terminate a publish. */
            }
        }
    });
    housekeeping = std::jthread([this](std::stop_token stop) {
        while (!stop.stop_requested()) {
            try {
                auto channels = store.list("channels");
                for (size_t i = 0; i < channels.size(); ++i) {
                    auto c = channels.at(i);
                    int months = c.get("autoCleanupOlderThanMonths").num(),
                        interval = c.get("autoCleanupIntervalMonths").num(1);
                    // I record precedenti senza toggle mantengono la vecchia impostazione per mesi.
                    if (c.get("autoCleanupEnabled").flag(months > 0) && interval > 0) {
                        auto last = c.get("lastCleanupEpoch").num();
                        double epoch = std::time(nullptr);
                        if (historyCleanupThreshold(-1, std::time_t(last)) <
                                historyCleanupThreshold(interval) &&
                            historyCleanup->tryStart(months, c.get("id").num())) {
                            auto updated = store.get("channels", c.get("id").num());
                            updated.set("lastCleanupEpoch", epoch);
                            store.save("channels", updated, c.get("id").num());
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
    airWatcher.request_stop();
    airWatcher.join();
    housekeeping.request_stop();
    housekeeping.join();
    historyCleanup.reset();
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
        // Lo storico conserva l'identità al momento della diretta, anche se il canale cambia.
        r.set("channelName", c.get("channelName"));
        r.set("platform", c.get("platform"));
        r.set("ffmpegSender", c.get("ffmpegSender").flag(true));
        if (!asksForStreamKey(c.get("platform").str(), c.get("platformStreamName").str(),
                              c.get("streamUrl").str()))
            r.set("streamKey", c.get("streamKey"));
        // Una scena scelta nel wizard non richiede la cartella playlist del canale.
        if (!r.has("files") && !r.has("scene") && r.get("sceneId").num() <= 0) {
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
        // Resolve an active platform default before falling back to another active setting.
        double activeId = 0;
        auto profiles = store.list("profiles");
        const auto wanted = streamPlatform(r.get("streamUrl").str(), r.get("platform").str());
        for (size_t i = 0; i < profiles.size(); ++i) {
            const auto candidate = profiles.at(i);
            if (!candidate.get("isActive").flag())
                continue;
            if (activeId == 0)
                activeId = candidate.get("id").num();
            if (wanted != StreamPlatform::Generic &&
                candidate.get("isDefaultConfiguration").flag() &&
                streamPlatform("", candidate.get("platform").str()) == wanted) {
                activeId = candidate.get("id").num();
                break;
            }
        }
        if (activeId == 0)
            throw std::runtime_error("Nessun profilo video e audio attivo");
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
        const double startIndex = r.get("startIndex").num();
        if (!std::isfinite(startIndex) || startIndex < 0 || startIndex != std::floor(startIndex) ||
            startIndex >= double(r.get("files").size()))
            throw std::runtime_error("Indice iniziale playlist non valido");
        for (size_t i = 0; i < r.get("files").size(); ++i)
            probe(r.get("files").at(i).str());
    }
    const double offset = r.get("offsetSeconds").num();
    if (!std::isfinite(offset) || offset < 0)
        throw std::runtime_error("Posizione di ripresa non valida");
    Json h;
    h.set("status", "STARTING");
    r.set("startedAt", now());
    h.set("startedAt", r.get("startedAt"));
    h.set("channelId", double(channel));
    h.set("channelName", r.get("channelName"));
    h.set("platform", r.get("platform"));
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
    for (auto& [id, s] : sessions) {
        auto snapshot = s->snapshot();
        snapshot.set("air", youtubeAir->state(id));
        a.push(snapshot);
    }
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
