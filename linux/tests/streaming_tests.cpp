#include "core.hpp"
#include "paced_output.hpp"
#include "rate_control.hpp"
#include "overlay_media.hpp"
#include "platform_lookup.hpp"
#include "async_logger.hpp"
#include "composition_preview.hpp"
#include "screen_capture.hpp"
#include "stream_platform.hpp"
#include "encoder_options.hpp"
#include <future>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
using namespace orbis;
using namespace std::chrono_literals;
static void expect(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}
int main() {
    auto directory = fs::temp_directory_path() / ("orbis-stream-engine-" + std::to_string(getpid()));
    try {
        fs::remove_all(directory);
        fs::create_directories(directory);
        {
            Store store(directory / "data");
            const auto profiles = store.list("profiles");
            expect(profiles.size() == 10, "ten Windows defaults");
            int high = 0;
            for (size_t i = 0; i < profiles.size(); ++i) {
                const auto p = profiles.at(i);
                validate("profiles", p);
                high += p.get("isDefaultConfiguration").flag();
                expect(p.get("isActive").flag(), "defaults active like Windows");
                if (p.get("platform").str() == "YouTube" && p.get("isDefaultConfiguration").flag())
                    expect(p.get("videoBitrate").num() == 6000 && p.get("videoWidth").num() == 1920,
                           "current Windows YouTube default");
            }
            expect(high == 5, "five high and five low defaults");
            auto custom = profiles.at(0).clone();
            custom.set("title", "My edited default");
            custom.set("videoBitrate", 1234.);
            store.save("profiles", custom, custom.get("id").num());
        }
        {
            Store reopened(directory / "data");
            expect(reopened.list("profiles").size() == 10, "no duplicate seeds on restart");
            expect(reopened.list("profiles").at(0).get("videoBitrate").num() == 1234,
                   "preserve user edits to defaults");
        }
        {
            const auto setting = Json::parse(R"({"preset":"llhq","videoSettingsOptions":[{"key":"rc","value":"cbr_hq"},{"key":"multipass","value":"qres"}]})");
            const auto options = encoderOptions(setting, "h264_nvenc");
            auto value = [&](const std::string& key) {
                for (size_t i = 0; i < options.size(); ++i)
                    if (options.at(i).get("key").str() == key) return options.at(i).get("value").str();
                return std::string{};
            };
            expect(value("preset") == "p7" && value("tune") == "ll" && value("rc") == "cbr", "FFmpeg 9 removed NVENC aliases translated");
            expect(value("multipass") == "qres", "explicit modern NVENC options win");
            const auto custom = Json::parse(R"([{"key":"x264-params","value":"nal-hrd=vbr:rc_lookahead=20"}])");
            const auto parameters = x264Parameters(custom, true);
            expect(parameters.find("nal-hrd=cbr") != std::string::npos && parameters.find("rc_lookahead=20") != std::string::npos && parameters.find("force-cfr=1") != std::string::npos, "YouTube CBR merged with manual x264 parameters");
            expect(profileSwitch(Json{}, "adaptiveBitrate", "adaptivebitrate"), "adaptive bitrate defaults on like Windows");
            expect(videoPixelFormat(Json::parse(R"({"pixelFormat":0})")) == "yuv420p", "Windows numeric pixel format");
            expect(encoderOptionSupported("libx264", "aq-mode"), "codec-specific AVOption accepted");
            expect(!encoderOptionSupported("libx264", "i") && !encoderOptionSupported("libx264", "filter_complex"), "codec options cannot replace input or composition");
            auto scene = Json::parse(R"({"width":64,"height":32,"items":[{"sourceKind":"CAMERA","sourceTarget":"/dev/video0"}]})");
            auto profile = Json::parse(R"({"videoCodecName":"h264_vaapi"})");
            const auto build = buildProducer(scene, profile, 0, {});
            expect(std::find(build.args.begin(), build.args.end(), "-vaapi_device") != build.args.end(), "VAAPI device initialized");
            const auto graph = std::find(build.args.begin(), build.args.end(), "-filter_complex");
            expect(graph != build.args.end() && (graph+1)->find("hwupload") != std::string::npos, "VAAPI uploads composed frames");
        }
        {
            std::stop_source cancel;
            auto command = std::async(std::launch::async, [&] {
                try { capture({"/bin/sleep", "10"}, 15, cancel.get_token()); return false; }
                catch (...) { return true; }
            });
            std::this_thread::sleep_for(30ms);
            cancel.request_stop();
            expect(command.wait_for(800ms) == std::future_status::ready && command.get(), "closing cancels platform requests and their child processes");
        }
        {
            ChangeNotifier notifier;
            auto since = notifier.version();
            notifier.raise();
            expect(notifier.wait(since, {}, 1s) == since+1, "late listener sees the change immediately");
            std::stop_source cancel;
            auto listener = std::async(std::launch::async, [&] { return notifier.wait(since+1, cancel.get_token(), 5s); });
            cancel.request_stop();
            expect(listener.wait_for(250ms) == std::future_status::ready, "notifier cancellation wakes its waiter");
        }
        {
            std::atomic<int> writes{0};
            std::atomic<bool> cancel{false};
            const auto started = std::chrono::steady_clock::now();
            PacedOutput output(100, 0, true, [&] { return cancel.load(); }, [] {},
                               [&](const FlvChunk&) { ++writes; }, [] {});
            for (uint64_t time : {0, 50, 100, 150, 200})
                output.push({{1}, time, true, {}});
            output.finish();
            const auto elapsed = std::chrono::steady_clock::now() - started;
            expect(writes == 5 && elapsed >= 190ms && elapsed < 2s, "media clock pacing");
            expect(output.snapshot().get("done").flag(), "publisher drains before done");
        }
        {
            std::atomic<bool> release{false}, entered{false};
            PacedOutput output(100, 0, false, [] { return false; }, [&] {
                entered = true;
                while (!release) std::this_thread::sleep_for(1ms);
            }, [](const FlvChunk&) {}, [] {});
            while (!entered) std::this_thread::sleep_for(1ms);
            output.push({{1}, 0, true, {}});
            output.push({{1}, 100, true, {}});
            auto blocked = std::async(std::launch::async, [&] { output.push({{1}, 200, true, {}}); });
            expect(blocked.wait_for(30ms) == std::future_status::timeout, "lead bounds producer");
            release = true;
            expect(blocked.wait_for(2s) == std::future_status::ready, "draining unblocks producer");
            output.finish();
        }
        {
            PacedOutput output(100, 0, false, [] { return false; }, [] {},
                [](const FlvChunk&) { throw std::runtime_error("simulated disconnect"); }, [] {});
            output.push({{1}, 0, true, {}});
            bool failed = false;
            try { output.finish(); } catch (const std::exception& e) {
                failed = std::string(e.what()) == "simulated disconnect";
            }
            expect(failed, "network errors propagate to session");
        }
        {
            std::atomic<bool> cancel{false};
            PacedOutput output(1000, 0, true, [&] { return cancel.load(); }, [] {},
                [](const FlvChunk&) {}, [] {});
            output.push({{1}, 0, true, {}});
            output.push({{1}, 60000, true, {}});
            const auto started = std::chrono::steady_clock::now();
            cancel = true;
            output.abort();
            expect(std::chrono::steady_clock::now() - started < 250ms, "cancel wakes pacing wait");
        }
        {
            RateControl rate(false, 1000);
            std::optional<int> reduced;
            int64_t time;
            for (time = 0; time < 40000 && !reduced; time += 200)
                reduced = rate.observe({time, time/2, time*200, 1000}, 4500, 128);
            expect(reduced && *reduced >= 2250 && *reduced <= 3825, "slow network lowers bounded bitrate");
            const int lower = *reduced;
            std::optional<int> recovered;
            const auto origin = time;
            for (; time < origin + 200000 && !recovered; time += 200)
                recovered = rate.observe({time, origin/2 + time-origin, time*500, 0}, 4500, 128);
            expect(recovered && *recovered > lower && *recovered <= 4500, "stable delivery probes recovery");
            RateControl encoder(false, 1000);
            for (time = 0; time < 60000; time += 200)
                expect(!encoder.observe({time, time/2, time*200, 0},4500,128), "encoder starvation is not congestion");
        }
        {
            expect(deliveryFor(StreamPlatform::Kick).uniformFormat, "Kick preserves one output format");
            expect(deliveryFor(StreamPlatform::YouTube).reconnectAttempts == 5, "YouTube retry policy");
            expect(deliveryFor(StreamPlatform::Twitch).audioSampleRate == 44100, "Twitch audio profile");
            const auto input = screenInput("x11-window:123@:1.0", 30);
            expect(std::find(input.begin(), input.end(), "-window_id") != input.end() && input.back() == ":1.0", "window capture input");
            const auto monitor = screenInput("x11-monitor:1920,0,1280,720@:0", 30);
            expect(std::find(monitor.begin(), monitor.end(), "1280x720") != monitor.end(), "monitor capture bounds");
            bool bad = false;
            try { screenInput("x11-window:1 -i secret@:0", 30); } catch (...) { bad = true; }
            expect(bad, "malformed capture targets rejected");
        }
        {
            Store store(directory / "preview-data");
            const auto png = directory / "red.png", frame = directory / "composition.jpg";
            capture({ffmpeg(), "-v", "error", "-y", "-f", "lavfi", "-i", "color=c=red:s=64x32", "-frames:v", "1", "-threads", "1", png.string()});
            auto scene = Json::parse(R"({"width":64,"height":32,"items":[{"sourceKind":"FILE","sourceTarget":"","x":0,"y":0,"width":64,"height":32}]})");
            scene.get("items").at(0).set("sourceTarget", png.string());
            CompositionPreview preview(store, frame);
            preview.update(scene);
            for (int i = 0; i < 100 && !fs::exists(frame); ++i)
                std::this_thread::sleep_for(30ms);
            expect(fs::exists(frame), "compositor renders a real preview before going live");
            const auto picture = probe(frame.string()).get("streams").at(0);
            expect(picture.get("width").num() == 64 && picture.get("height").num() == 32, "preview keeps canvas aspect");
        }
        {
            auto alpha = Json::parse(R"({"format":{"format_name":"matroska,webm"},"streams":[{"codec_type":"video","codec_name":"vp9","tags":{"alpha_mode":"1"}}]})");
            expect(OverlayMedia::of(alpha).decoder == "libvpx-vp9", "VP9 alpha decoder");
            auto still = Json::parse(R"({"format":{"format_name":"png_pipe"},"streams":[{"codec_type":"video","codec_name":"png"}]})");
            expect(!OverlayMedia::of(still).loop, "static PNG decoded once");
            auto anim = Json::parse(R"({"format":{"format_name":"webp_anim"},"streams":[{"codec_type":"video","codec_name":"webp_anim"}]})");
            expect(OverlayMedia::of(anim).nativeWebpLoop, "FFmpeg 9 WebP demuxer");
            std::string bytes("RIFF\x12\0\0\0WEBPANIM\x06\0\0\0\0\0\0\0\x02\0", 26);
            const auto original = directory / "finite.webp";
            writeFile(original, bytes);
            fs::path copy;
            {
                WebpOverlayInput input(original);
                copy = input.path;
                expect(copy != original && fs::exists(copy), "session-owned WebP copy");
                const auto content = readFile(copy);
                expect(content.size() == bytes.size() && content[24] == 0 && content[25] == 0,
                       "only ANIM loop count patched");
                expect(readFile(original) == bytes, "original WebP unchanged");
            }
            expect(!fs::exists(copy), "WebP copy cleaned on producer destruction");
        }
        {
            expect(youtubeCandidate(R"(var ytInitialData = {"contentId":"abc123_-xyz","contentType":"LOCKUP_CONTENT_TYPE_VIDEO"};</script>)") == "abc123_-xyz", "YouTube lockup lookup");
            const auto live = lookupYouTube("@test", [](const std::string& url) {
                return url.ends_with("/streams") ? R"({"videoId":"abc123_-xyz","thumbnail":{}})" : R"({"isLiveContent":true,"isLiveNow":true})";
            });
            expect(live.get("air").str() == "LIVE" && live.has("embed"), "verified YouTube broadcast");
            expect(youtubeEnded(R"({"isLiveContent":true})") == true, "ended public broadcast");
            expect(!youtubeEnded("consent page").has_value(), "unknown HTML cannot prove ending");
            YouTubeAirState state;
            expect(state.since(1,100) == 100 && state.since(1,200) == 100, "grace belongs to whole live");
            Json offline; offline.set("air", "OFFLINE");
            state.observe(1, offline);
            expect(!state.state(1).get("offAir").flag(), "one missing lookup is not off air");
            state.observe(1, offline);
            expect(state.state(1).get("offAir").flag(), "two confirmed misses warn");
            state.dismiss(1);
            expect(!state.state(1).get("offAir").flag(), "dismiss warning per live");
            state.observe(1, live);
            state.observe(1, offline); state.observe(1, offline);
            expect(state.state(1).get("offAir").flag(), "found live resets warning dismissal");
            expect(!state.ending(1, true) && !state.ending(1, {}) && !state.ending(1,true) && state.ending(1,true),
                   "two consecutive positive endings required");
        }
        {
            const auto logs = directory / "logs";
            fs::create_directories(logs);
            const auto old = logs / "application-2000-01-01.log";
            writeFile(old, "old");
            fs::last_write_time(old, fs::file_time_type::clock::now() - 96h);
            AsyncLogger logger(logs);
            for (int i = 0; i < 100; ++i)
                logger.write("test", std::to_string(i), logs / "session.log");
            logger.flush();
            expect(readFile(logs/"session.log").find("[test] 99") != std::string::npos, "flush persists queued diagnostics");
            expect(!fs::exists(old), "three-day application log retention");
        }
        fs::remove_all(directory);
        std::cout << "Delivery pacing/backpressure/cancel, adaptive bitrate, Windows defaults, overlays, YouTube air state and logging: OK\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << " (fixtures: " << directory << ")\n";
        return 1;
    }
}
