#pragma once
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>

namespace orbis {
class AsyncLogger {
    struct Entry { std::filesystem::path file; std::string text; };
    std::filesystem::path root;
    std::mutex mutex;
    std::condition_variable changed;
    std::deque<Entry> queue;
    size_t dropped = 0, queuedBytes = 0;
    bool stopping = false, writing = false;
    std::thread writer;
    void run();
  public:
    explicit AsyncLogger(std::filesystem::path directory);
    ~AsyncLogger();
    void write(const std::string& component, const std::string& text,
               const std::filesystem::path& sessionFile = {});
    void flush();
};
} // namespace orbis
