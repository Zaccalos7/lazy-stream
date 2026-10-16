#pragma once
#include "core.hpp"

namespace orbis {
struct OverlayMedia {
    bool loop = false, nativeWebpLoop = false;
    std::string decoder;
    static OverlayMedia of(const Json& probe);
};

// Owns a private loopable WebP copy; destruction follows the producer using it.
class WebpOverlayInput {
    fs::path directory;

  public:
    fs::path path;
    explicit WebpOverlayInput(const fs::path& source);
    ~WebpOverlayInput();
    WebpOverlayInput(const WebpOverlayInput&) = delete;
};
} // namespace orbis
