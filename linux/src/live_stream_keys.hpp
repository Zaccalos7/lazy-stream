#pragma once

#include <string>

namespace orbis {
bool asksForStreamKey(const std::string& platform, const std::string& platformStreamName,
                      const std::string& streamUrl);
std::string standInStreamKey(const std::string& channelName);
bool isStandInStreamKey(const std::string& streamKey);
} // namespace orbis
