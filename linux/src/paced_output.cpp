#include "paced_output.hpp"
#include <algorithm>
#include <cstdarg>
#include <stdexcept>
#ifdef ORBIS_FFMPEG9
extern "C" {
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/log.h>
}
#endif

namespace orbis {
namespace {
int64_t ticks() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
}

PacedOutput::PacedOutput(int64_t lead, int64_t burst, bool pace,
                         std::function<bool()> cancel, std::function<void()> initialize,
                         std::function<void(const FlvChunk&)> sink, std::function<void()> finalize)
    : maxLead(lead), preroll(burst), pacing(pace), cancelled(std::move(cancel)),
      open(std::move(initialize)), complete(std::move(finalize)), write(std::move(sink)) {
    sender = std::thread([this] { run(); });
}

PacedOutput::~PacedOutput() { abort(); }

void PacedOutput::push(FlvChunk chunk) {
    std::unique_lock lock(mutex);
    auto full = [&] {
        return !queue.empty() && (queuedBytes + chunk.bytes.size() > 32 * 1024 * 1024 ||
            queue.size() >= 4096 ||
            int64_t(chunk.timestamp - std::min(chunk.timestamp, queue.front().timestamp)) > maxLead);
    };
    while (full() && failure.empty() && !aborted && !cancelled())
        condition.wait_for(lock, std::chrono::milliseconds(50));
    if (!failure.empty())
        throw std::runtime_error(failure);
    if (aborted || cancelled())
        return;
    if (ended)
        throw std::runtime_error("Write after publisher completion");
    lastQueued = std::max(lastQueued, chunk.timestamp);
    queuedBytes += chunk.bytes.size();
    peakBytes = std::max(peakBytes, queuedBytes);
    queue.push_back(std::move(chunk));
    condition.notify_all();
}

void PacedOutput::run() {
    try {
        open();
        auto epoch = std::chrono::steady_clock::now();
        auto lastWrite = epoch;
        uint64_t previousStamp = 0;
        bool clockStarted = false;
        for (;;) {
            FlvChunk chunk;
            bool backlogged = false;
            {
                std::unique_lock lock(mutex);
                while (queue.empty() && !ended && !aborted && !cancelled())
                    condition.wait_for(lock, std::chrono::milliseconds(50));
                if (aborted || cancelled())
                    break;
                if (queue.empty() && ended)
                    break;
                chunk = std::move(queue.front());
                queue.pop_front();
                queuedBytes -= chunk.bytes.size();
                backlogged = !queue.empty();
                condition.notify_all();
            }
            if (chunk.media && pacing) {
                const auto now = std::chrono::steady_clock::now();
                if (!clockStarted) {
                    epoch = now - std::chrono::milliseconds(chunk.timestamp);
                    clockStarted = true;
                }
                auto due = epoch + std::chrono::milliseconds(int64_t(chunk.timestamp) - preroll);
                // Preserve the media clock. A backlog drains at 2x, never as an uncontrolled burst.
                if (backlogged && chunk.timestamp > uint64_t(preroll) &&
                    now > due + std::chrono::milliseconds(50)) {
                    const auto delta = chunk.timestamp - std::min(previousStamp, chunk.timestamp);
                    due = std::max(due, lastWrite + std::chrono::milliseconds(delta / 2));
                }
                std::unique_lock lock(mutex);
                while (!aborted && !cancelled() && std::chrono::steady_clock::now() < due)
                    condition.wait_until(lock, std::min(due, std::chrono::steady_clock::now() +
                                                       std::chrono::milliseconds(50)));
                if (aborted || cancelled())
                    break;
            }
            write(chunk);
            lastWrite = std::chrono::steady_clock::now();
            previousStamp = chunk.timestamp;
            {
                std::lock_guard lock(mutex);
                sentBytes += chunk.bytes.size();
                if (chunk.media) {
                    if (chunk.context)
                        context = chunk.context;
                    published = true;
                    if (chunk.timestamp > sentTimestamp)
                        lastAdvance = ticks();
                    sentTimestamp = std::max(sentTimestamp, chunk.timestamp);
                }
            }
        }
        complete();
    } catch (const std::exception& error) {
        std::lock_guard lock(mutex);
        failure = error.what();
    }
    std::lock_guard lock(mutex);
    completed = true;
    condition.notify_all();
}

void PacedOutput::finish() {
    {
        std::lock_guard lock(mutex);
        ended = true;
        condition.notify_all();
    }
    if (sender.joinable())
        sender.join();
    check();
}

void PacedOutput::abort() {
    {
        std::lock_guard lock(mutex);
        aborted = true;
        condition.notify_all();
    }
    if (sender.joinable())
        sender.join();
}

void PacedOutput::check() {
    std::lock_guard lock(mutex);
    if (!failure.empty())
        throw std::runtime_error(failure);
}

DeliverySample PacedOutput::sample() {
    std::lock_guard lock(mutex);
    return {ticks(), int64_t(sentTimestamp), int64_t(sentBytes),
            queue.empty() ? 0 : int64_t(lastQueued - std::min(lastQueued, sentTimestamp))};
}

Json PacedOutput::snapshot() {
    std::lock_guard lock(mutex);
    Json result;
    result.set("published", published);
    result.set("timestamp", double(sentTimestamp));
    result.set("bytes", double(sentBytes));
    result.set("queuedBytes", double(queuedBytes));
    result.set("peakBytes", double(peakBytes));
    result.set("queuedTags", double(queue.size()));
    result.set("leadMilliseconds", double(queue.empty() ? 0 : lastQueued - std::min(lastQueued, sentTimestamp)));
    result.set("lastAdvance", double(lastAdvance));
    result.set("done", completed);
    if (context)
        result.set("context", context->clone());
    return result;
}

#ifdef ORBIS_FFMPEG9
namespace {
thread_local std::function<void(const std::string&)>* threadLog = nullptr;
void nativeLog(void* context, int level, const char* format, va_list args) {
    if (level > AV_LOG_INFO)
        return;
    if (!threadLog)
        return;
    char text[8192];
    int prefix = 1;
    av_log_format_line2(context, level, format, args, text, sizeof(text), &prefix);
    (*threadLog)(text);
}
std::string avError(int error) {
    char text[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(error, text, sizeof(text));
    return text;
}
}
#endif

struct NativePublisher::Impl {
    std::string url;
    std::function<bool()> cancelled;
    std::function<void(const std::string&)> log;
    int timeout;
    std::atomic<int64_t> deadline{0};
#ifdef ORBIS_FFMPEG9
    AVIOContext* output = nullptr;
#endif
};

NativePublisher::NativePublisher(std::string url, std::function<bool()> cancelled,
                                 std::function<void(const std::string&)> log, int timeout)
    : impl(std::make_unique<Impl>()) {
    impl->url = std::move(url);
    impl->cancelled = std::move(cancelled);
    impl->log = std::move(log);
    impl->timeout = timeout;
}
NativePublisher::~NativePublisher() { close(); }

bool NativePublisher::available() {
#ifdef ORBIS_FFMPEG9
    return true;
#else
    return false;
#endif
}

void NativePublisher::open() {
#ifdef ORBIS_FFMPEG9
    static std::once_flag initialized;
    std::call_once(initialized, [] { avformat_network_init(); av_log_set_callback(nativeLog); });
    threadLog = &impl->log;
    impl->deadline = ticks() + impl->timeout * 1000;
    AVIOInterruptCB interrupt{[](void* opaque) {
        auto self = static_cast<Impl*>(opaque);
        return int(self->cancelled() || ticks() > self->deadline.load());
    }, impl.get()};
    AVDictionary* options = nullptr;
    av_dict_set(&options, "rw_timeout", "15000000", 0);
    av_dict_set(&options, "rtmp_live", "live", 0);
    av_dict_set(&options, "rtmp_flush_interval", "1", 0);
    if (impl->url.starts_with("rtmps://")) {
        av_dict_set(&options, "tls_verify", "1", 0);
        av_dict_set(&options, "ca_file", "/etc/ssl/certs/ca-certificates.crt", 0);
    }
    // Facebook keys can contain '/': the app and playpath must remain separate.
    if (const auto path = impl->url.find("/rtmp/"); path != std::string::npos) {
        av_dict_set(&options, "rtmp_app", "rtmp", 0);
        av_dict_set(&options, "rtmp_playpath", impl->url.substr(path + 6).c_str(), 0);
        av_dict_set(&options, "rtmp_tcurl", (impl->url.substr(0, path) + "/rtmp").c_str(), 0);
    }
    const int result = avio_open2(&impl->output, impl->url.c_str(), AVIO_FLAG_WRITE,
                                  &interrupt, &options);
    av_dict_free(&options);
    if (result < 0)
        throw std::runtime_error("Native publisher connect: " + avError(result));
    impl->log(std::string("FFmpeg ") + av_version_info() + " libavformat publish accepted");
#else
    throw std::runtime_error("Native publisher requires the FFmpeg 9.0.2 development libraries");
#endif
}

void NativePublisher::write(const FlvChunk& chunk) {
#ifdef ORBIS_FFMPEG9
    threadLog = &impl->log;
    impl->deadline = ticks() + impl->timeout * 1000;
    avio_write(impl->output, chunk.bytes.data(), int(chunk.bytes.size()));
    avio_flush(impl->output);
    if (impl->output->error < 0)
        throw std::runtime_error("Native publisher write: " + avError(impl->output->error));
#else
    (void)chunk;
#endif
}

void NativePublisher::close() {
#ifdef ORBIS_FFMPEG9
    if (impl->output) {
        threadLog = &impl->log;
        impl->deadline = ticks() + 3000;
        const int result = avio_closep(&impl->output);
        if (result < 0)
            impl->log("Native publisher close: " + avError(result));
        threadLog = nullptr;
    }
#endif
}
} // namespace orbis
