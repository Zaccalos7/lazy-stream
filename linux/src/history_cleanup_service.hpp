#pragma once
#include "core.hpp"

namespace orbis {
// Soglie di LiveHistoryCleanupService.cs: -1 tutto, 0 ieri, valori positivi mesi di calendario.
std::string historyCleanupThreshold(int months, std::time_t reference = std::time(nullptr));

// Il lavoro e il progresso appartengono al servizio: la pagina può cambiare mentre la pulizia
// continua.
class LiveHistoryCleanupService {
    Store& store;
    std::mutex startMutex, progressMutex;
    Json state;
    std::jthread worker;
    void run(int months, long channel, std::stop_token stop);
    void publish(const Json& progress);

  public:
    explicit LiveHistoryCleanupService(Store& store);
    ~LiveHistoryCleanupService();
    bool tryStart(int months, long channel = 0);
    Json progress();
};
} // namespace orbis
