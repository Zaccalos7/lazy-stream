#include "live_stream_keys.hpp"
#include "stream_platform.hpp"

#include <algorithm>
#include <cctype>
#include <string_view>
#include <utility>

namespace orbis {
namespace {
constexpr std::string_view prefix = "asked-at-every-live:";

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool tiktokName(std::string value) {
    value = trim(std::move(value));
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value == "tiktok";
}
} // namespace

// Come LiveStreamKeys.IsAskedFor, il nome pubblico o il dominio ingest bastano a richiedere una
// chiave nuova; il campo platform Linux copre inoltre la selezione esplicita nel modulo.
bool asksForStreamKey(const std::string& platform, const std::string& platformStreamName,
                      const std::string& streamUrl) {
    return tiktokName(platform) || tiktokName(platformStreamName) || isTikTokIngest(streamUrl);
}

// Il segnaposto distingue configurazioni TikTok senza conservare una chiave già scaduta.
std::string standInStreamKey(const std::string& channelName) {
    return std::string(prefix) + trim(channelName);
}

bool isStandInStreamKey(const std::string& streamKey) {
    return streamKey.starts_with(prefix);
}
} // namespace orbis
