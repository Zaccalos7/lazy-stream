#pragma once
#include "core.hpp"
#include <functional>
#include <optional>

namespace orbis {
Json platformPlayer(const Json& channel);
Json platformPlayer(const Json& channel, std::stop_token stop);
std::string youtubeCandidate(const std::string& html);
std::optional<bool> youtubeEnded(const std::string& html);
Json lookupYouTube(const std::string& name,
                   const std::function<std::string(const std::string&)>& fetch);

class YouTubeAirState {
    struct Live {
        int misses = 0, failures = 0, endings = 0, absent = 0;
        bool dismissed = false;
        std::string video;
        int64_t since = -1;
    };

    std::mutex mutex;
    std::map<long, Live> lives;

  public:
    int64_t since(long id, int64_t now);
    void observe(long id, const Json& result);
    bool ending(long id, std::optional<bool> ended);
    void missing(const std::vector<long>& present);
    void dismiss(long id);
    Json state(long id);
};
} // namespace orbis
