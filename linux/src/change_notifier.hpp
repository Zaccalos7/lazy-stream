#pragma once
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <stop_token>
#include <chrono>
namespace orbis {
class ChangeNotifier {
    std::mutex mutex;
    std::condition_variable_any changed;
    uint64_t revision = 0;
  public:
    uint64_t version() { std::lock_guard lock(mutex); return revision; }
    void raise() { std::lock_guard lock(mutex); ++revision; changed.notify_all(); }
    uint64_t wait(uint64_t since, std::stop_token stop, std::chrono::milliseconds timeout) {
        std::unique_lock lock(mutex);
        changed.wait_for(lock, stop, timeout, [&] { return revision != since; });
        return revision;
    }
};
} // namespace orbis
