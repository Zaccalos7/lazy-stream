#pragma once
#include "core.hpp"
#include <condition_variable>

namespace orbis {
// Preview uses the same composition graph, without streaming, H.264 encoding or audio playback.
class CompositionPreview {
    Store& store;
    fs::path frame;
    std::mutex mutex;
    std::condition_variable changed;
    Json scene;
    uint64_t revision = 0;
    bool stopped = false;
    std::thread worker;
    void run();

  public:
    CompositionPreview(Store& store, fs::path frame);
    ~CompositionPreview();
    void update(const Json& scene);
};
} // namespace orbis
