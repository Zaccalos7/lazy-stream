#include "platform_lookup.hpp"
#include "stream_platform.hpp"
#include "async_logger.hpp"
#include <fstream>

namespace orbis {
void Studio::dismissAir(long id) { youtubeAir->dismiss(id); store.changes.raise(); }

void Studio::checkYouTube(std::stop_token stop) {
    std::vector<long> present;
    const auto snapshots = live();
    const auto nowSeconds = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    for (size_t i = 0; i < snapshots.size(); ++i) {
        if (stop.stop_requested()) return;
        const auto snapshot = snapshots.at(i);
        if (snapshot.get("status").str() != "LIVE")
            continue;
        const long id = snapshot.get("id").num();
        Json channel;
        try { channel = store.get("channels", snapshot.get("channelId").num()); }
        catch (...) { continue; }
        if (channel.get("platform").str() != "YouTube" &&
            detectStreamPlatform(channel.get("streamUrl").str()) != StreamPlatform::YouTube)
            continue;
        present.push_back(id);
        if (nowSeconds - youtubeAir->since(id, nowSeconds) < 90)
            continue;
        Json found;
        try { found = platformPlayer(channel, stop); }
        catch (const std::exception& error) {
            found.set("air", "UNVERIFIED");
            found.set("reason", error.what());
        }
        if (stop.stop_requested()) return;
        const auto before = youtubeAir->state(id).dump();
        youtubeAir->observe(id, found);
        if (before != youtubeAir->state(id).dump()) store.changes.raise();
        const auto state = youtubeAir->state(id);
        const auto video = state.get("videoId").str();
        std::optional<bool> ended;
        if (found.get("air").str() == "OFFLINE" && !video.empty()) {
            try {
                ended = youtubeEnded(capture({"curl", "--fail", "--silent", "--location",
                    "--compressed", "--max-time", "8", "https://www.youtube.com/watch?v=" + video}, 10, stop));
            } catch (...) {}
        }
        if (stop.stop_requested()) return;
        store.logger->write("youtube-air", "live=" + std::to_string(id) + " state=" + found.get("air").str() +
            " video=" + video + " reason=" + redactStreamText(found.get("reason").str()),
            store.root / "logs" / (std::to_string(id) + ".session.log"));
        // Absence and transient network errors never stop a session. Require the previously
        // watched broadcast to be positively reported ended on two consecutive passes.
        if (youtubeAir->ending(id, ended))
            session(id)->stop("La trasmissione è terminata su YouTube");
    }
    youtubeAir->missing(present);
}
} // namespace orbis
