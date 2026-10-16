#pragma once

#include <string>

namespace orbis {
enum class StreamPlatform { Generic, Twitch, YouTube, Kick, Facebook };

struct StreamDeliveryProfile {
    int connectSeconds = 30, stallSeconds = 30, reconnectAttempts = 3;
    int maxLeadMilliseconds = 1000, prerollMilliseconds = 0, audioSampleRate = 48000;
    bool buffered = false, uniformFormat = false, constantBitrate = false;
};

struct StreamFrame {
    int width, height;
    double frameRate;
};

// The first resolved output fixes a shared connection; small/portrait unnamed clips start at HD.
StreamFrame frameOfLive(StreamFrame first, bool namedResolution);

StreamDeliveryProfile deliveryFor(StreamPlatform platform);
StreamPlatform detectStreamPlatform(const std::string& outputUrl);
StreamPlatform streamPlatform(const std::string& outputUrl, const std::string& declared);
// Riproduce la normalizzazione RTMPS di StreamPlatform.cs senza alterare altre piattaforme.
std::string normalizeIngestUrl(const std::string& outputUrl);
bool supportedPlatform(const std::string& name);
} // namespace orbis
