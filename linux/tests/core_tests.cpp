#include "core.hpp"
#include "history_cleanup_service.hpp"
#include "live_rows.hpp"
#include "live_stream_keys.hpp"
#include "scene_service.hpp"
#include "stream_platform.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
using namespace orbis;

void expect(bool ok, const char* message) {
    if (!ok)
        throw std::runtime_error(message);
}

std::vector<unsigned char> segment(unsigned ts) {
    std::vector<unsigned char> v = {'F', 'L', 'V', 1, 5, 0, 0, 0, 9, 0,    0, 0, 0, 9, 0,
                                    0,   2,   0,   0, 0, 0, 0, 0, 0, 0x17, 1, 0, 0, 0, 13};
    v[17] = (ts >> 16) & 255;
    v[18] = (ts >> 8) & 255;
    v[19] = ts & 255;
    v[20] = ts >> 24;
    return v;
}

int main() {
    try {
        auto j = Json::parse("{\"items\":[{\"value\":2}]}");
        j.get("items").at(0).set("value", 3.0);
        expect(j.get("items").at(0).get("value").num() == 3, "JSON mutation");
        expect(Json::parse("27").num() == 27, "numeric codec option roundtrip");
        expect(Json::parse("true").flag(), "boolean scalar roundtrip");
        expect(Json::parse("null").num(77) == 77, "nullable dimension roundtrip");
        {
            auto scene = Json::parse(
                R"({"width":320,"height":180,"frameRate":15,"items":[{"sourceKind":"CAMERA","sourceTarget":"/dev/video0"}]})");
            auto profile = Json::parse(
                R"({"videoWidth":null,"videoHeight":null,"frameRate":null,"encoderQuality":"high","videoCodecName":"libx264","lowLatency":false,"tune":"film"})");
            auto arguments = buildProducer(scene, profile, 0, {}).args;
            auto selected = [&](const std::string& option) {
                auto found = std::find(arguments.begin(), arguments.end(), option);
                expect(found != arguments.end() && found + 1 != arguments.end(),
                       "encoder option present");
                return *(found + 1);
            };
            expect(selected("-preset") == "faster", "Windows x264 high quality");
            expect(selected("-tune") == "film", "manual tune with low latency disabled");
            expect(selected("-framerate") == "15", "keep source FPS");
            profile.set("videoCodecName", "h264_nvenc");
            profile.set("encoderQuality", "light");
            arguments = buildProducer(scene, profile, 0, {}).args;
            expect(selected("-preset") == "p1", "Windows NVENC light quality");
            profile.set("videoCodecName", "h264_qsv");
            profile.set("encoderQuality", "balanced");
            arguments = buildProducer(scene, profile, 0, {}).args;
            expect(selected("-preset") == "medium", "Windows QSV balanced quality");
        }
        bool bad = false;
        try {
            Json::parse("{} bad");
        } catch (...) {
            bad = true;
        }
        expect(bad, "JSON trailing bytes");
        expect(naturalLess("clip2.mp4", "clip10.mp4"), "natural order");
        expect(naturalLess("clip999999999999999999999.mp4", "clip1000000000000000000000.mp4"),
               "large numbers");
        expect(normalizeIngestUrl("rtmp://a.rtmp.youtube.com/live2/key") ==
                   "rtmps://a.rtmps.youtube.com/live2/key",
               "YouTube primary RTMPS");
        expect(normalizeIngestUrl("rtmps://b.rtmp.youtube.com:443/live2/key") ==
                   "rtmps://b.rtmps.youtube.com:443/live2/key",
               "YouTube backup RTMPS");
        expect(normalizeIngestUrl("rtmp://other-rtmp.youtube.com/live/key") ==
                   "rtmp://other-rtmp.youtube.com/live/key",
               "unrelated host unchanged");
        expect(normalizeIngestUrl("rtmps://example.test/live/key") ==
                   "rtmps://example.test/live/key",
               "other platforms unchanged");
        expect(isTikTokIngest("rtmps://push-rtmp-1.tiktokcdn-eu.com/live"),
               "regional TikTok ingest");
        expect(!isTikTokIngest("rtmp://evil-tiktokcdn.com/live"), "TikTok domain boundary");
        expect(detectStreamPlatform("rtmp://fa723fc1b171.global-contribute.live-video.net/live") ==
                   StreamPlatform::Kick,
               "Kick IVS endpoint before Twitch network");
        expect(detectStreamPlatform("rtmp://ingest.global-contribute.live-video.net/live") ==
                   StreamPlatform::Twitch,
               "Twitch shared network");
        expect(detectStreamPlatform("rtmps://a.rtmps.youtube.com/live") == StreamPlatform::YouTube,
               "YouTube ingest detection");
        expect(detectStreamPlatform("rtmps://live-api-s.facebook.com/live") ==
                   StreamPlatform::Facebook,
               "Facebook ingest detection");
        expect(detectStreamPlatform("https://a.rtmps.youtube.com/live") == StreamPlatform::Generic,
               "non-RTMP URL ignored");
        const auto sender =
            buildPublisher("rtmps://live-api-s.facebook.com:443/rtmp/raw/key?token=abc");
        auto option = [&](const std::string& name) {
            auto found = std::find(sender.begin(), sender.end(), name);
            expect(found != sender.end() && found + 1 != sender.end(), "publisher option missing");
            return *(found + 1);
        };
        expect(option("-rtmp_app") == "rtmp", "Facebook application independent of key slashes");
        expect(option("-rtmp_playpath") == "raw/key?token=abc", "Facebook raw playpath preserved");
        expect(option("-rtmp_tcurl") == "rtmps://live-api-s.facebook.com:443/rtmp",
               "Facebook tcUrl");
        expect(option("-probesize") == "4194304", "complete AVC and AAC headers before publish");
        Json diagnosticRequest;
        diagnosticRequest.set("streamKey", "raw/key?token=abc");
        const auto safe =
            redactStreamText("playpath raw/key?token=abc RTMPS://host/rtmp/key", diagnosticRequest);
        expect(safe.find("token=abc") == std::string::npos &&
                   safe.find("host/rtmp") == std::string::npos,
               "redact separate playpath and case-insensitive URL");
        expect(asksForStreamKey("Custom", "tiktok", "rtmp://example.test/live"),
               "TikTok public name requires fresh key");
        expect(asksForStreamKey("Custom", "", "rtmp://push.tiktokcdn.com/live"),
               "TikTok ingest requires fresh key");
        expect(standInStreamKey("  channel  ") == "asked-at-every-live:channel",
               "TikTok key stand-in");
        expect(isStandInStreamKey(standInStreamKey("channel")), "TikTok key not publishable");
        using namespace std::chrono;
        const auto reference = system_clock::to_time_t(sys_days{year{2024} / 3 / 31} + hours{12});
        expect(historyCleanupThreshold(1, reference) == "2024-02-29T12:00:00Z",
               "cleanup leap month clamp");
        expect(historyCleanupThreshold(12, reference) == "2023-03-31T12:00:00Z",
               "cleanup calendar year");
        expect(historyCleanupThreshold(0, reference) == "2024-03-30T12:00:00Z",
               "cleanup yesterday");
        expect(historyCleanupThreshold(-1, reference) == "2024-03-31T12:00:00Z",
               "cleanup everything");
        bad = false;
        try {
            historyCleanupThreshold(-2, reference);
        } catch (const std::runtime_error&) {
            bad = true;
        }
        expect(bad, "cleanup invalid threshold rejected");
        FlvRelay relay;
        relay.segment();
        auto v = segment(1000);
        std::vector<unsigned char> out;
        for (auto c : v) {
            auto chunk = relay.feed(&c, 1, false);
            out.insert(out.end(), chunk.begin(), chunk.end());
        }
        expect(out.size() == v.size(), "fragmented FLV");
        expect(relay.timestamp() == 0, "zero epoch");
        relay.segment();
        out = relay.feed(v.data(), v.size(), false);
        expect(out.size() == v.size() - 13, "single header");
        expect(relay.timestamp() == 1, "continuous FLV");
        auto dir = fs::temp_directory_path() / ("orbis-core-" + std::to_string(getpid()));
        fs::create_directories(dir);
        {
            Process failed;
            failed.start({"/bin/false"});
            while (failed.running())
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            expect(failed.wait() == 1, "retain exit status after nonblocking process check");
            Process diagnostic;
            const auto log = dir / "redacted.log";
            diagnostic.start(
                {"python3", "-c",
                 "import sys,time; sys.stderr.write('playpath raw/key?'); sys.stderr.flush(); "
                 "time.sleep(.02); sys.stderr.write('token=abc\\n')"},
                -1, log, true, [diagnosticRequest](const std::string& line) {
                    return redactStreamText(line, diagnosticRequest);
                });
            diagnostic.wait();
            diagnostic.stop();
            std::ifstream diagnosticFile(log);
            const std::string diagnosticText{std::istreambuf_iterator<char>(diagnosticFile), {}};
            expect(diagnosticText.find("token=abc") == std::string::npos &&
                       diagnosticText.find("[chiave nascosta]") != std::string::npos,
                   "redact stderr secrets split across pipe reads before writing");
            expect((fs::status(log).permissions() & fs::perms::group_read) == fs::perms::none,
                   "diagnostic file private");
        }
        {
            Store store(dir);
            {
                auto request = Json::parse(
                    R"({"profile":{},"scene":{"width":320,"height":180,"items":[{"sourceKind":"CAMERA","sourceTarget":"/dev/video0","volume":100}]}})");
                Session session(store, request, 0);
                auto snapshot = session.snapshot();
                snapshot.get("scene").set("width", 640.);
                snapshot.get("videoResults").at(0).set("status", "ERROR");
                const auto independent = session.snapshot();
                expect(independent.get("scene").get("width").num() == 320,
                       "snapshot scene detached from worker");
                expect(independent.get("videoResults").at(0).get("status").str() == "OFFLINE",
                       "snapshot results detached from worker");
            }
            Json c;
            c.set("channelName", "Test");
            c.set("streamUrl", "rtmp://example.test/live");
            c.set("streamKey", "secret");
            auto id = store.save("channels", c);
            expect(store.get("channels", id).get("streamKey").str() == "secret", "persistence");
            auto exported = store.exportData();
            expect(exported.get("channels").at(0).get("streamKey").str().empty(), "export secrets");
            auto dup = exported.clone();
            dup.get("channels").push(dup.get("channels").at(0));
            bad = false;
            try {
                store.importData(dup);
            } catch (...) {
                bad = true;
            }
            expect(bad, "duplicate import rejected");
            expect(store.list("channels").size() == 1, "import transaction rollback");
            store.importData(exported);
            expect(store.get("channels", id).get("streamKey").str().empty(), "import cleared key");
            auto historic = Json::parse(
                R"({"status":"STOPPED","startedAt":"2020-01-01T00:00:00Z","channelId":1,"channelName":"Original name","platform":"Twitch","position":1,"request":{"files":["first.mp4","second.mp4"]}})");
            const auto historicId = store.save("history", historic);
            auto currentSessions = Json::array();
            currentSessions.push(
                Json::parse("{\"id\":" + std::to_string(historicId) +
                            ",\"status\":\"LIVE\",\"position\":0,\"offsetSeconds\":2}"));
            auto rows = liveRows(store, currentSessions);
            expect(rows.size() == 1 && rows.at(0).get("running").flag() &&
                       rows.at(0).get("title").str() == "first.mp4" &&
                       rows.at(0).get("channelName").str() == "Original name",
                   "live rows merge session and historical identity");
            auto offlineRows = liveRows(store, Json::array());
            expect(offlineRows.at(0).get("status").str() == "STOPPED" &&
                       offlineRows.at(0).get("title").str() == "second.mp4" &&
                       offlineRows.at(0).get("isPlaylist").flag(),
                   "live rows persist after app restart");
            store.remove("history", historicId);

            // CheckSlots e CheckLayout devono applicare le stesse regole anche senza editor web.
            Json layout;
            layout.set("name", "Layout");
            layout.set("isLayout", true);
            layout.set("width", 320.0);
            layout.set("height", 180.0);
            layout.set("items", Json::array());
            bad = false;
            try {
                saveScene(store, layout);
            } catch (const std::runtime_error&) {
                bad = true;
            }
            expect(bad, "empty layout rejected");
            Json slot;
            slot.set("sourceKind", "FILE");
            slot.set("sourceTarget", "/temporary/source.mp4");
            slot.set("x", 0.0);
            slot.set("y", 0.0);
            slot.set("width", 320.0);
            slot.set("height", 180.0);
            layout.get("items").push(slot);
            const auto layoutId = saveScene(store, layout);
            expect(
                store.get("scenes", layoutId).get("items").at(0).get("sourceTarget").str().empty(),
                "layout source target cleared");
            auto outside = layout.clone();
            outside.get("items").at(0).set("x", 1.0);
            bad = false;
            try {
                saveScene(store, outside);
            } catch (const std::runtime_error&) {
                bad = true;
            }
            expect(bad, "outside-canvas slot rejected");

            writeFile(dir / "clip.mp4", "fixture");
            Json scene = layout.clone();
            scene.set("name", "Scena");
            scene.set("isLayout", false);
            scene.get("items").at(0).set("sourceTarget", (dir / "clip.mp4").string());
            auto sceneId = saveScene(store, scene);
            auto numericKind = scene.clone();
            numericKind.get("items").at(0).set("sourceKind", "0");
            auto numericId = saveScene(store, numericKind);
            expect(store.get("scenes", numericId).get("items").at(0).get("sourceKind").str() ==
                       "FILE",
                   "Windows numeric source kind normalized");
            auto duplicate = scene.clone();
            duplicate.get("items").push(duplicate.get("items").at(0));
            bad = false;
            try {
                saveScene(store, duplicate);
            } catch (const std::runtime_error&) {
                bad = true;
            }
            expect(bad, "duplicate source rejected");
            auto missing = scene.clone();
            missing.get("items").at(0).set("sourceTarget", (dir / "missing.mp4").string());
            bad = false;
            try {
                saveScene(store, missing);
            } catch (const std::runtime_error&) {
                bad = true;
            }
            expect(bad, "missing file rejected");

            Json record;
            Json request;
            request.set("scene", store.get("scenes", sceneId));
            record.set("request", request);
            store.save("history", record);
            auto revised = scene.clone();
            revised.set("name", "Scena modificata");
            auto revisionId = saveScene(store, revised, sceneId);
            expect(revisionId != sceneId, "on-air scene copied on edit");
            expect(store.get("scenes", sceneId).get("name").str() == "Scena",
                   "on-air scene unchanged");
            bad = false;
            try {
                ensureSceneCanDelete(store, sceneId);
            } catch (const std::runtime_error&) {
                bad = true;
            }
            expect(bad, "on-air scene cannot be deleted");

            writeFile(dir / "overlays" / "badge.png", "fixture");
            Json overlay;
            overlay.set("sourceKind", "OVERLAY");
            overlay.set("sourceTarget", (dir / "overlays" / "badge.png").string());
            overlay.set("x", 0.0);
            overlay.set("y", 0.0);
            overlay.set("width", 50.0);
            overlay.set("height", 50.0);
            auto withOverlay = layout.clone();
            withOverlay.get("items").push(overlay);
            saveScene(store, withOverlay);
            expect(overlayInUse(store, dir / "overlays" / "badge.png"),
                   "used overlay cannot be deleted");
            overlay.set("sourceTarget", (dir / "clip.mp4").string());
            bad = false;
            try {
                validateSceneForSave(Json::parse("{\"name\":\"Invalid\",\"isLayout\":true,"
                                                 "\"width\":320,\"height\":180,\"items\":[" +
                                                 overlay.dump() + "]}"),
                                     dir);
            } catch (const std::runtime_error&) {
                bad = true;
            }
            expect(bad, "overlay outside library rejected");
        }
        {
            Store store(dir / "cleanup");
            auto old = Json::parse(
                R"({"status":"ENDED","channelId":1,"startedAt":"2020-01-01T00:00:00Z","request":{"files":["first.mp4","second.mp4"]}})");
            const auto oldId = store.save("history", old);
            auto scene = old.clone();
            scene.set("request", Json::parse(R"({"scene":{"name":"Old scene"}})"));
            store.save("history", scene);
            auto otherChannel = old.clone();
            otherChannel.set("channelId", 2.0);
            store.save("history", otherChannel);
            auto live = old.clone();
            live.set("status", "LIVE");
            const auto liveId = store.save("history", live);
            live.set("status", "STARTING");
            store.save("history", live);
            live.set("status", "RECONNECTING");
            const auto reconnectingId = store.save("history", live);
            expect(!store.removeHistoryIfInactive(reconnectingId, old.get("startedAt").str()),
                   "cleanup protects reconnecting status");
            auto fresh = old.clone();
            fresh.set("startedAt", now());
            store.save("history", fresh);
            expect(!store.removeHistoryIfInactive(liveId, old.get("startedAt").str()),
                   "cleanup protects active status");
            expect(!store.removeHistoryIfInactive(oldId, "2019-01-01T00:00:00Z"),
                   "cleanup rejects stale identity");
            LiveHistoryCleanupService cleanup(store);
            expect(cleanup.tryStart(0, 1), "cleanup worker started");
            for (int i = 0; i < 200 && cleanup.progress().get("running").flag(); ++i)
                std::this_thread::sleep_for(milliseconds{10});
            const auto progress = cleanup.progress();
            expect(!progress.get("running").flag() && progress.get("error").str().empty(),
                   "cleanup worker completed");
            expect(progress.get("deletedLives").num() == 2 &&
                       progress.get("deletedVideos").num() == 3 &&
                       progress.get("totalVideos").num() == 3,
                   "cleanup counts playlist and scene snapshots");
            expect(store.list("history").size() == 5,
                   "cleanup preserves active, fresh and other channel history");
        }
        {
            Store recovered(dir / "cleanup");
            const auto history = recovered.list("history");
            for (size_t i = 0; i < history.size(); ++i)
                expect(history.at(i).get("status").str() != "RECONNECTING",
                       "reconnecting history recovered after restart");
        }
        for (const auto [folder, schema] : {
                 std::pair{"foreign", "CREATE TABLE setting(id INTEGER); PRAGMA user_version=1"},
                 std::pair{"future", "PRAGMA user_version=99"},
             }) {
            const auto path = dir / folder;
            fs::create_directories(path);
            sqlite3* db = nullptr;
            expect(sqlite3_open((path / "stream.db").c_str(), &db) == SQLITE_OK,
                   "open incompatible database fixture");
            expect(sqlite3_exec(db, schema, nullptr, nullptr, nullptr) == SQLITE_OK,
                   "create incompatible database fixture");
            sqlite3_close(db);
            bad = false;
            try {
                Store incompatible(path);
            } catch (const std::runtime_error&) {
                bad = true;
            }
            expect(bad, "incompatible database rejected");
        }
        {
            Store beforeRestart(dir / "video-recovery");
            beforeRestart.save(
                "history",
                Json::parse(
                    R"({"status":"LIVE","videoResults":[{"status":"LIVE"},{"status":"ENDED"}]})"));
        }
        {
            Store afterRestart(dir / "video-recovery");
            const auto result = afterRestart.list("history").at(0).get("videoResults");
            expect(result.at(0).get("status").str() == "INTERRUPTED",
                   "recover active video after restart");
            expect(result.at(1).get("status").str() == "ENDED",
                   "preserve completed video after restart");
        }
        fs::remove_all(dir);
        expect(capture({"printf", "%s", "$(touch /tmp/orbis-must-not-exist)"}) ==
                   "$(touch /tmp/orbis-must-not-exist)",
               "no shell interpolation");
        std::cout << "JSON, SQLite, export/import, natural sorting, processes, FLV: OK\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
