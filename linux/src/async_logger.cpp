#include "async_logger.hpp"
#include "core.hpp"
#include <fstream>

namespace orbis {
AsyncLogger::AsyncLogger(fs::path directory) : root(std::move(directory)) {
    fs::create_directories(root);
    writer = std::thread([this] { run(); });
}
AsyncLogger::~AsyncLogger() {
    { std::lock_guard lock(mutex); stopping = true; changed.notify_all(); }
    writer.join();
}
void AsyncLogger::write(const std::string& component, const std::string& message,
                        const fs::path& sessionFile) {
    const auto stamp = now();
    const auto text = stamp + " [" + component + "] " + message.substr(0, 65536) + "\n";
    std::lock_guard lock(mutex);
    const size_t needed = sessionFile.empty() ? 1 : 2;
    if (queue.size() + needed > 10000 || queuedBytes + text.size() * needed > 8 * 1024 * 1024) {
        ++dropped;
        return;
    }
    queuedBytes += text.size() * needed;
    queue.push_back({root / ("application-" + stamp.substr(0, 10) + ".log"), text});
    if (!sessionFile.empty())
        queue.push_back({sessionFile, text});
    changed.notify_all();
}
void AsyncLogger::flush() {
    std::unique_lock lock(mutex);
    changed.wait(lock, [&] { return queue.empty() && !writing; });
}
void AsyncLogger::run() {
    std::string rotated;
    for (;;) {
        std::deque<Entry> batch;
        size_t lost;
        {
            std::unique_lock lock(mutex);
            changed.wait(lock, [&] { return stopping || !queue.empty(); });
            if (queue.empty() && stopping)
                break;
            batch.swap(queue);
            queuedBytes = 0;
            writing = true;
            lost = dropped;
            dropped = 0;
        }
        try {
            const auto day = now().substr(0, 10);
            if (rotated != day) {
                for (auto& entry : fs::directory_iterator(root)) {
                    if (entry.path().filename().string().starts_with("application-") &&
                        fs::file_time_type::clock::now() - entry.last_write_time() > std::chrono::hours(72)) {
                        std::error_code ignored;
                        fs::remove(entry.path(), ignored);
                    }
                }
                rotated = day;
            }
            for (const auto& entry : batch) {
                std::ofstream file(entry.file, std::ios::app);
                if (lost) {
                    file << now() << " [logger] dropped " << lost << " entries: log queue full\n";
                    lost = 0;
                }
                file << entry.text;
                std::error_code ignored;
                fs::permissions(entry.file, fs::perms::owner_read | fs::perms::owner_write, ignored);
            }
        } catch (...) { /* Diagnostics cannot stop a live or terminate its writer thread. */ }
        { std::lock_guard lock(mutex); writing = false; changed.notify_all(); }
    }
}
} // namespace orbis
