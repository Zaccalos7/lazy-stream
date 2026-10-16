#include "history_cleanup_service.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace orbis {
// Usa mesi di calendario e limita il giorno alla fine del mese, come DateTime.AddMonths di Windows.
std::string historyCleanupThreshold(int months, std::time_t reference) {
    if (months < -1 || months > 120)
        throw std::runtime_error("Soglia pulizia: -1, 0 oppure 1–120 mesi");
    using namespace std::chrono;
    sys_seconds instant{seconds{reference}};
    if (months == 0)
        instant -= days{1};
    else if (months > 0) {
        auto dayStart = floor<days>(instant);
        year_month_day date{dayStart};
        auto target = date.year() / date.month() - std::chrono::months{months};
        auto lastDay = year_month_day{target / last}.day();
        auto adjusted = target / std::min(date.day(), lastDay);
        instant = sys_days{adjusted} + (instant - dayStart);
    }
    auto cutoff = system_clock::to_time_t(instant);
    std::tm tm{};
    gmtime_r(&cutoff, &tm);
    std::ostringstream result;
    result << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return result.str();
}

// Uno stato esplicito permette al browser di leggere il progresso anche prima del primo lavoro.
LiveHistoryCleanupService::LiveHistoryCleanupService(Store& store) : store(store) {
    state.set("running", false);
    state.set("totalVideos", 0.0);
    state.set("deletedVideos", 0.0);
    state.set("deletedLives", 0.0);
    state.set("currentVideo", "");
    state.set("error", "");
}

// Attende il worker prima di distruggere lo store a cui fa riferimento.
LiveHistoryCleanupService::~LiveHistoryCleanupService() {
    worker.request_stop();
    if (worker.joinable())
        worker.join();
}

// Restituisce una copia: json-c usa riferimenti condivisi, quindi il chiamante non deve vedere
// mutazioni.
Json LiveHistoryCleanupService::progress() {
    std::lock_guard lock(progressMutex);
    return state.clone();
}

// Pubblica una copia sotto lock per dare una vista coerente ai diversi client e allo scheduler.
void LiveHistoryCleanupService::publish(const Json& progress) {
    std::lock_guard lock(progressMutex);
    state = progress.clone();
}

// Serializza gli avvii e rifiuta il secondo lavoro; non tiene il lock del progresso durante join.
bool LiveHistoryCleanupService::tryStart(int months, long channel) {
    historyCleanupThreshold(months);
    std::lock_guard lock(startMutex);
    auto current = progress();
    if (current.get("running").flag())
        return false;
    if (worker.joinable())
        worker.join();
    Json initial;
    initial.set("running", true);
    initial.set("startedAt", now());
    initial.set("totalVideos", 0.0);
    initial.set("deletedVideos", 0.0);
    initial.set("deletedLives", 0.0);
    initial.set("currentVideo", "");
    initial.set("error", "");
    publish(initial);
    try {
        worker = std::jthread(
            [this, months, channel](std::stop_token stop) { run(months, channel, stop); });
    } catch (const std::exception& e) {
        initial.set("running", false);
        initial.set("error", e.what());
        publish(initial);
        throw;
    }
    return true;
}

// Conta i video degli snapshot; elimina solo record ancora inattivi, senza toccare i media
// originali.
void LiveHistoryCleanupService::run(int months, long channel, std::stop_token stop) {
    auto current = progress();
    try {
        const auto cutoff = historyCleanupThreshold(months);
        std::vector<Json> candidates;
        auto history = store.list("history");
        for (size_t i = 0; i < history.size(); ++i) {
            auto item = history.at(i);
            auto status = item.get("status").str();
            if (status == "LIVE" || status == "STARTING" || status == "RECONNECTING" ||
                item.get("startedAt").str().empty() || item.get("startedAt").str() >= cutoff ||
                (channel && item.get("channelId").num() != channel))
                continue;
            candidates.push_back(item);
            current.set("totalVideos",
                        current.get("totalVideos").num() +
                            std::max<size_t>(1, item.get("request").get("files").size()));
        }
        publish(current);
        for (const auto& item : candidates) {
            if (stop.stop_requested())
                break;
            auto request = item.get("request");
            current.set("currentVideo",
                        request.get("scene").get("name").str(request.get("files").at(0).str(
                            "Live " + std::to_string(long(item.get("id").num())))));
            publish(current);
            if (store.removeHistoryIfInactive(item.get("id").num(), item.get("startedAt").str())) {
                current.set("deletedLives", current.get("deletedLives").num() + 1);
                current.set("deletedVideos", current.get("deletedVideos").num() +
                                                 std::max<size_t>(1, request.get("files").size()));
                publish(current);
            }
        }
    } catch (const std::exception& e) {
        current.set("error", e.what());
    }
    current.set("running", false);
    current.set("currentVideo", "");
    publish(current);
}
} // namespace orbis
