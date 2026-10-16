#pragma once

#include <string>

namespace orbis {
enum class StreamPlatform { Generic, Twitch, YouTube, Kick, Facebook, TikTok };
struct StreamDeliveryProfile {
    int connectSeconds = 30, stallSeconds = 30, reconnectAttempts = 3;
    int maxLeadMilliseconds = 1000, prerollMilliseconds = 0, audioSampleRate = 48000;
    bool buffered = false, uniformFormat = false, constantBitrate = false, portrait = false;
};
StreamDeliveryProfile deliveryFor(StreamPlatform platform);
StreamPlatform detectStreamPlatform(const std::string& outputUrl);
StreamPlatform streamPlatform(const std::string& outputUrl, const std::string& declared);
// Riproduce la normalizzazione RTMPS di StreamPlatform.cs senza alterare altre piattaforme.
std::string normalizeIngestUrl(const std::string& outputUrl);
bool isTikTokIngest(const std::string& outputUrl);
} // namespace orbis
