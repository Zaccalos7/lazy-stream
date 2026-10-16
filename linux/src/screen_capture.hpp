#pragma once
#include "core.hpp"
namespace orbis {
Json screenSources();
std::vector<std::string> screenInput(const std::string& target, double fps);
} // namespace orbis
