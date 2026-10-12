#include "live_rows.hpp"
#include <algorithm>

namespace orbis {
// Unisce lo snapshot persistente con i contatori correnti: le sessioni finite non spariscono dalla pagina.
Json liveRows(Store& store, const Json& sessions) {
    auto history = store.list("history"), channels = store.list("channels");
    std::vector<Json> rows;
    for (size_t i = 0; i < history.size(); ++i) {
        auto row = history.at(i).clone();
        const auto request = row.get("request");
        Json current;
        for (size_t j = 0; j < sessions.size(); ++j)
            if (sessions.at(j).get("id").num() == row.get("id").num()) {
                current = sessions.at(j);
                for (const auto key : {"status", "position", "offsetSeconds", "outputKbps", "error"})
                    row.set(key, current.get(key));
                break;
            }
        auto status = row.get("status").str();
        row.set("running", current.has("id") && (status == "LIVE" || status == "STARTING" || status == "RECONNECTING"));
        // Gli storici precedenti al port di MainLiveHistory non avevano ancora questi due campi.
        std::string channelName = row.get("channelName").str(request.get("channelName").str());
        std::string platform = row.get("platform").str(request.get("platform").str());
        for (size_t j = 0; j < channels.size(); ++j) {
            auto channel = channels.at(j);
            if (channel.get("id").num() == row.get("channelId").num()) {
                if (channelName.empty()) channelName = channel.get("channelName").str();
                if (platform.empty()) platform = channel.get("platform").str("Custom");
                break;
            }
        }
        row.set("channelName", channelName);
        row.set("platform", platform);
        const auto files = request.get("files"), scene = request.get("scene");
        const bool isScene = scene.isObject();
        const size_t total = isScene ? scene.get("items").size() : files.size();
        const size_t position = total ? std::min<size_t>(std::max(0.0, row.get("position").num()), total - 1) : 0;
        row.set("position", double(position));
        row.set("total", double(total));
        row.set("isScene", isScene);
        row.set("isPlaylist", !isScene && files.size() > 1);
        const auto path = files.at(position).str();
        row.set("title", isScene ? scene.get("name").str() : fs::path(path).filename().string());
        row.set("source", isScene ? scene.get("name").str() : path);
        rows.push_back(row);
    }
    std::sort(rows.begin(), rows.end(), [](const Json& a, const Json& b) {
        auto left = a.get("startedAt").str(), right = b.get("startedAt").str();
        return left != right ? left > right : a.get("id").num() > b.get("id").num();
    });
    auto result = Json::array();
    for (const auto& row : rows) result.push(row);
    return result;
}
} // namespace orbis
