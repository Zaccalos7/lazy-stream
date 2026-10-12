#include "stream_platform.hpp"

#include <algorithm>
#include <cctype>
#include <string_view>

namespace orbis {
namespace {
std::string ingestHost(const std::string& url) {
    const auto schemeEnd = url.find("://");
    if (schemeEnd == std::string::npos)
        return {};
    auto scheme = url.substr(0, schemeEnd);
    std::transform(scheme.begin(), scheme.end(), scheme.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (scheme != "rtmp" && scheme != "rtmps")
        return {};
    const auto start = schemeEnd + 3;
    const auto end = url.find_first_of(":/?#", start);
    auto host = url.substr(start, end - start);
    if (host.empty() || host.find('@') != std::string::npos || host.front() == '[')
        return {};
    std::transform(host.begin(), host.end(), host.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return host;
}

bool isOrUnder(const std::string& host, std::string_view domain) {
    return host == domain ||
           (host.size() > domain.size() &&
            host.compare(host.size() - domain.size(), domain.size(), domain) == 0 &&
            host[host.size() - domain.size() - 1] == '.');
}

bool isIvsChannelEndpoint(const std::string& host) {
    constexpr std::string_view suffix = ".global-contribute.live-video.net";
    if (host.size() != 12 + suffix.size() || !host.ends_with(suffix))
        return false;
    return std::all_of(host.begin(), host.begin() + 12,
                       [](unsigned char c) { return std::isxdigit(c); });
}
} // namespace

// L'ordine coincide con StreamPlatforms.Detect: gli endpoint IVS di Kick appartengono alla rete
// Twitch, quindi vanno riconosciuti prima di classificare il dominio live-video.net.
StreamPlatform detectStreamPlatform(const std::string& outputUrl) {
    const auto host = ingestHost(outputUrl);
    if (host.empty())
        return StreamPlatform::Generic;
    if (isOrUnder(host, "kick.com") || isIvsChannelEndpoint(host))
        return StreamPlatform::Kick;
    if (isOrUnder(host, "twitch.tv") || isOrUnder(host, "live-video.net"))
        return StreamPlatform::Twitch;
    if (isOrUnder(host, "youtube.com"))
        return StreamPlatform::YouTube;
    if (isOrUnder(host, "facebook.com"))
        return StreamPlatform::Facebook;
    for (std::string_view domain :
         {"tiktokcdn.com", "tiktokcdn-eu.com", "tiktokcdn-us.com", "tiktokv.com", "tiktok.com"})
        if (isOrUnder(host, domain))
            return StreamPlatform::TikTok;
    return StreamPlatform::Generic;
}

// YouTube accetta TLS sull'host rtmps.youtube.com: converte insieme schema e host, conservando
// lettera dell'ingest, porta, applicazione e chiave come fa StreamPlatform.NormalizeIngestUrl.
std::string normalizeIngestUrl(const std::string& outputUrl) {
    const auto host = ingestHost(outputUrl);
    if (!isOrUnder(host, "rtmp.youtube.com"))
        return outputUrl;
    const auto schemeEnd = outputUrl.find("://");
    const size_t hostEnd = outputUrl.find_first_of(":/?#", schemeEnd + 3);
    constexpr std::string_view oldHost = "rtmp.youtube.com";
    const auto suffix = hostEnd == std::string::npos ? std::string{} : outputUrl.substr(hostEnd);
    return "rtmps://" + host.substr(0, host.size() - oldHost.size()) + "rtmps.youtube.com" + suffix;
}

// Riconosce i domini regionali elencati da StreamPlatforms.Detect senza fidarsi del suffisso di
// una stringa arbitraria (per esempio evil-tiktokcdn.com).
bool isTikTokIngest(const std::string& outputUrl) {
    return detectStreamPlatform(outputUrl) == StreamPlatform::TikTok;
}

} // namespace orbis
