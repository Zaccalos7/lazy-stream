#pragma once
#include "core.hpp"
#include <condition_variable>
#include <deque>
#include <functional>

namespace orbis {
struct DeliverySample {
    int64_t ticks = 0, media = 0, bytes = 0, lead = 0;
};

// One bounded compressed-packet queue per publish. No decoded frames cross this queue.
class PacedOutput {
    std::mutex mutex;
    std::condition_variable condition;
    std::deque<FlvChunk> queue;
    size_t queuedBytes = 0, peakBytes = 0;
    uint64_t lastQueued = 0, sentTimestamp = 0, sentBytes = 0;
    int64_t lastAdvance = 0;
    bool ended = false, aborted = false, completed = false, published = false;
    std::string failure;
    std::shared_ptr<const Json> context;
    int64_t maxLead, preroll;
    bool pacing;
    std::function<bool()> cancelled;
    std::function<void()> open, complete;
    std::function<void(const FlvChunk&)> write;
    std::thread sender;
    void run();

  public:
    PacedOutput(int64_t maxLead, int64_t preroll, bool pacing,
                std::function<bool()> cancelled, std::function<void()> open,
                std::function<void(const FlvChunk&)> write, std::function<void()> complete);
    ~PacedOutput();
    void push(FlvChunk chunk);
    void finish();
    void abort();
    void check();
    DeliverySample sample();
    Json snapshot();
};

// Public libavformat AVIO API: RTMP handshake, chunking, acknowledgments and TLS stay in FFmpeg.
class NativePublisher {
    struct Impl;
    std::unique_ptr<Impl> impl;
  public:
    NativePublisher(std::string url, std::function<bool()> cancelled,
                    std::function<void(const std::string&)> log, int timeoutSeconds = 30);
    ~NativePublisher();
    static bool available();
    void open();
    void write(const FlvChunk& chunk);
    void close();
};
} // namespace orbis
