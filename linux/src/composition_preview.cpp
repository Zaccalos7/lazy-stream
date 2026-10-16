#include "composition_preview.hpp"
#include "async_logger.hpp"
#include <cmath>
namespace orbis {
CompositionPreview::CompositionPreview(Store& store, fs::path frame)
    : store(store), frame(std::move(frame)), worker([this] { run(); }) {}
CompositionPreview::~CompositionPreview() {
    { std::lock_guard lock(mutex); stopped = true; changed.notify_all(); }
    worker.join();
    std::error_code ignored;
    fs::remove(frame, ignored);
}
void CompositionPreview::update(const Json& value) {
    std::lock_guard lock(mutex);
    scene = value.clone();
    ++revision;
    changed.notify_all();
}
void CompositionPreview::run() {
    uint64_t running = 0;
    Process process;
    Build build;
    for (;;) {
        Json next;
        {
            std::unique_lock lock(mutex);
            changed.wait_for(lock, std::chrono::milliseconds(100), [&] { return stopped || running != revision; });
            if (stopped) break;
            if (running == revision) continue;
            running = revision;
            next = scene.clone();
        }
        process.stop();
        build = {};
        if (!next.get("items").size()) continue;
        try {
            const double width = next.get("width").num(1920), height = next.get("height").num(1080);
            Json profile;
            const double scale = std::min(1., 640. / std::max(width, height));
            profile.set("videoWidth", std::max(16., std::floor(width * scale / 2) * 2));
            profile.set("videoHeight", std::max(16., std::floor(height * scale / 2) * 2));
            profile.set("previewOnly", true);
            auto items = next.get("items");
            for (size_t i = 0; i < items.size(); ++i)
                items.at(i).set("loop", true);
            build = buildProducer(next, profile, 0, frame);
            process.start(build.args, -1, store.root / "logs" / "composition-preview.log", false,
                          [](const std::string& message) { return redactStreamText(message); });
        } catch (const std::exception& error) {
            store.logger->write("composition-preview", redactStreamText(error.what()));
        }
    }
    process.stop();
}
} // namespace orbis
