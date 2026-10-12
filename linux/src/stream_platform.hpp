#pragma once

#include <string>

namespace orbis {
enum class StreamPlatform { Generic, Twitch, YouTube, Kick, Facebook, TikTok };
StreamPlatform detectStreamPlatform(const std::string& outputUrl);
// Riproduce la normalizzazione RTMPS di StreamPlatform.cs senza alterare altre piattaforme.
std::string normalizeIngestUrl(const std::string& outputUrl);
bool isTikTokIngest(const std::string& outputUrl);
} // namespace orbis
