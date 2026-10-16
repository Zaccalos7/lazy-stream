#pragma once
#include "core.hpp"

namespace orbis {
Json encoderOptions(const Json& profile, const std::string& encoder);
bool profileSwitch(const Json& profile, const std::string& field, const std::string& option);
std::string x264Parameters(const Json& options, bool constantBitrate);
} // namespace orbis

namespace orbis {
std::string videoEncoder(const Json& profile);
std::string audioEncoder(const Json& profile);
std::string videoPixelFormat(const Json& profile);
bool encoderOptionSupported(const std::string& encoder, const std::string& option);
} // namespace orbis
