#include "core.hpp"
#include <fstream>
#include <sstream>
#include <unistd.h>

namespace orbis {
// Legge CPU e memoria da /proc; i campioni successivi permettono il calcolo CPU, con 100% pari a un
// core.
Json processMetrics(long pid) {
    static std::mutex mutex;
    std::lock_guard lock(mutex);

    struct Previous {
        double ticks;
        std::chrono::steady_clock::time_point time;
    };

    static std::map<long, Previous> previous;
    Json out;
    out.set("pid", double(pid));
    std::ifstream stat("/proc/" + std::to_string(pid) + "/stat");
    std::string text;
    std::getline(stat, text);
    auto end = text.rfind(')');
    if (end == std::string::npos)
        return out;
    std::istringstream s(text.substr(end + 2));
    std::vector<std::string> fields;
    std::string f;
    while (s >> f)
        fields.push_back(f);
    if (fields.size() < 22)
        return out;
    double ticks = std::stod(fields[11]) + std::stod(fields[12]);
    auto current = std::chrono::steady_clock::now();
    auto found = previous.find(pid);
    double cpu = -1;
    if (found != previous.end()) {
        double elapsed = std::chrono::duration<double>(current - found->second.time).count();
        if (elapsed > 0.01 && ticks >= found->second.ticks)
            cpu = 100 * (ticks - found->second.ticks) / sysconf(_SC_CLK_TCK) / elapsed;
    }
    previous[pid] = {ticks, current};
    if (previous.size() > 1024)
        previous.erase(previous.begin());
    out.set("cpu", cpu);
    out.set("threads", std::stod(fields[17]));
    out.set("rssBytes", std::stod(fields[21]) * sysconf(_SC_PAGESIZE));
    out.set("virtualBytes", std::stod(fields[20]));
    return out;
}
} // namespace orbis
