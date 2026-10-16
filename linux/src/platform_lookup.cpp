#include "platform_lookup.hpp"
#include "stream_platform.hpp"
#include <algorithm>
#include <cctype>
#include <regex>
#include <chrono>

namespace orbis {
Json platformPlayer(const Json& channel) { return platformPlayer(channel, {}); }

Json platformPlayer(const Json& channel, std::stop_token stop) {
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
        static std::mutex cacheMutex;
        static std::map<std::string, std::pair<Json, std::chrono::steady_clock::time_point>> cache;
        std::unique_lock lock(cacheMutex, std::defer_lock);
        while (!lock.try_lock()) {
            if (stop.stop_requested()) throw std::runtime_error("Operazione annullata");
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        const auto now = std::chrono::steady_clock::now();
        const auto cached = cache.find(name);
        if (cached != cache.end() && now - cached->second.second <
            std::chrono::seconds(cached->second.first.get("air").str() == "LIVE" ? 45 : 10))
            return cached->second.first.clone();
        auto found = lookupYouTube(name, [stop](const std::string& page) {
            return capture({"curl", "--fail", "--silent", "--show-error", "--location",
                            "--compressed", "--max-time", "8", page}, 10, stop);
        });
        if (stop.stop_requested()) throw std::runtime_error("Operazione annullata");
        cache[name] = {found.clone(), now};
        if (cache.size() > 256)
            cache.erase(cache.begin());
        return found;
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
std::string youtubeCandidate(const std::string& html) {
    std::string scope = html;
    auto start = scope.find("var ytInitialData = ");
    if (start != std::string::npos) {
        start += 20;
        const auto end = scope.find(";</script>", start);
        scope = scope.substr(start, end == std::string::npos ? end : end - start);
    }
    std::smatch match;
    for (const auto& pattern : {
            R"rx("contentId"\s*:\s*"([A-Za-z0-9_-]{11})"\s*,\s*"contentType"\s*:\s*"LOCKUP_CONTENT_TYPE_VIDEO")rx",
            R"rx("videoId"\s*:\s*"([A-Za-z0-9_-]{11})"\s*,\s*"thumbnail")rx"})
        if (std::regex_search(scope, match, std::regex(pattern)))
            return match[1].str();
    return {};
}

std::optional<bool> youtubeEnded(const std::string& html) {
    if (html.empty())
        return {};
    if (std::regex_search(html, std::regex(R"rx("isLive(?:Now)?"\s*:\s*true)rx")))
        return false;
    if (std::regex_search(html, std::regex(R"rx("isLiveContent"\s*:\s*true)rx")))
        return true;
    return {};
}

Json lookupYouTube(const std::string& name,
                   const std::function<std::string(const std::string&)>& fetch) {
    if (!std::regex_match(name, std::regex("[@A-Za-z0-9_.-]{1,120}")))
        throw std::runtime_error("Nome pubblico non valido");
    Json result;
    const auto channel = name.starts_with("UC") ? "https://www.youtube.com/channel/" + name
        : "https://www.youtube.com/" + (name.starts_with("@") ? name : "@" + name);
    result.set("publicUrl", channel + "/live");
    try {
        const auto streams = fetch(channel + "/streams");
        if (streams.empty())
            throw std::runtime_error("Empty YouTube response");
        const auto candidate = youtubeCandidate(streams);
        if (!candidate.empty()) {
            const auto watch = fetch("https://www.youtube.com/watch?v=" + candidate);
            if (watch.empty())
                throw std::runtime_error("Empty YouTube watch response");
            if (youtubeEnded(watch) == false) {
                result.set("videoId", candidate);
                result.set("publicUrl", "https://www.youtube.com/watch?v=" + candidate);
                result.set("embed", "https://www.youtube.com/embed/" + candidate +
                    "?autoplay=1&mute=1&playsinline=1&origin=http%3A%2F%2Flocalhost");
                result.set("air", "LIVE");
                return result;
            }
        }
        result.set("air", "OFFLINE");
    } catch (const std::exception& error) {
        result.set("air", "UNVERIFIED");
        result.set("reason", error.what());
    }
    return result;
}

int64_t YouTubeAirState::since(long id, int64_t now) {
    std::lock_guard lock(mutex);
    auto& live = lives[id];
    live.absent = 0;
    if (live.since < 0)
        live.since = now;
    return live.since;
}
void YouTubeAirState::observe(long id, const Json& result) {
    std::lock_guard lock(mutex);
    auto& live = lives[id];
    if (result.get("air").str() == "LIVE") {
        live.misses = live.failures = live.endings = 0;
        live.dismissed = false;
        live.video = result.get("videoId").str(live.video);
    } else if (result.get("air").str() == "OFFLINE") {
        ++live.misses;
        live.failures = 0;
    } else
        ++live.failures;
}
bool YouTubeAirState::ending(long id, std::optional<bool> ended) {
    std::lock_guard lock(mutex);
    auto& live = lives[id];
    live.endings = ended == true ? live.endings + 1 : 0;
    return live.endings >= 2;
}
void YouTubeAirState::missing(const std::vector<long>& present) {
    std::lock_guard lock(mutex);
    for (auto it = lives.begin(); it != lives.end();) {
        if (std::find(present.begin(), present.end(), it->first) == present.end() && ++it->second.absent >= 2)
            it = lives.erase(it);
        else
            ++it;
    }
}
void YouTubeAirState::dismiss(long id) {
    std::lock_guard lock(mutex);
    if (auto found = lives.find(id); found != lives.end())
        found->second.dismissed = true;
}
Json YouTubeAirState::state(long id) {
    std::lock_guard lock(mutex);
    Json result;
    const auto found = lives.find(id);
    if (found == lives.end())
        return result;
    const auto& live = found->second;
    result.set("offAir", !live.dismissed && live.misses >= 2);
    result.set("unverified", !live.dismissed && live.failures >= 2);
    result.set("videoId", live.video);
    return result;
}
} // namespace orbis
