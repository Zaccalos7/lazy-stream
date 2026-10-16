#include "live_rows.hpp"
#include "stream_platform.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>

namespace orbis {
namespace {
std::string lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

// La piattaforma nota di una riga, come la nomina Windows: dall'URL di ingresso quando lo
// riconosce, altrimenti dal nome scelto sul canale. Le piattaforme senza un marchio proprio
// (Custom, Rumble, X sul motore Linux) restano senza.
std::string knownPlatformOf(const std::string& streamUrl, const std::string& platform) {
    switch (detectStreamPlatform(streamUrl)) {
    case StreamPlatform::Twitch:
        return "twitch";
    case StreamPlatform::YouTube:
        return "youtube";
    case StreamPlatform::Kick:
        return "kick";
    case StreamPlatform::Facebook:
        return "facebook";
    case StreamPlatform::TikTok:
        return "tiktok";
    default:
        break;
    }
    const auto named = lower(platform);
    for (const char* known : {"twitch", "youtube", "kick", "facebook", "tiktok"})
        if (named == known)
            return named;
    return {};
}

// Un fermo immagine resta in onda finché non si riprende la diretta: è l'unico caso in cui la riga
// mostra "Riprendi diretta". Un video, anche a schermo intero, finisce da solo.
bool takeoverHolds(const Json& takeover) {
    if (!takeover.has("scene"))
        return false;
    const auto extension = lower(fs::path(takeover.get("mediaName").str()).extension().string());
    const bool image = extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
                       extension == ".gif" || extension == ".webp" || extension == ".bmp";
    return image && takeover.get("durationSeconds").num() <= 0;
}
} // namespace

// Unisce lo snapshot persistente con i contatori correnti: le sessioni finite non spariscono dalla
// pagina.
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
                for (const auto key :
                     {"status", "position", "offsetSeconds", "outputKbps", "error", "videoResults"})
                    row.set(key, current.get(key));
                break;
            }
        auto status = row.get("status").str();
        row.set("running", current.has("id") && (status == "LIVE" || status == "STARTING" ||
                                                 status == "RECONNECTING"));
        // Gli storici precedenti al port di MainLiveHistory non avevano ancora questi due campi.
        std::string channelName = row.get("channelName").str(request.get("channelName").str());
        std::string platform = row.get("platform").str(request.get("platform").str());
        std::string streamUrl = request.get("streamUrl").str();
        for (size_t j = 0; j < channels.size(); ++j) {
            auto channel = channels.at(j);
            if (channel.get("id").num() == row.get("channelId").num()) {
                if (channelName.empty())
                    channelName = channel.get("channelName").str();
                if (platform.empty())
                    platform = channel.get("platform").str("Custom");
                if (streamUrl.empty())
                    streamUrl = channel.get("streamUrl").str();
                break;
            }
        }
        row.set("channelName", channelName);
        row.set("platform", platform);
        // La colonna azioni mostra il marchio della piattaforma su cui la riga è in onda, non il
        // nome: è ciò a cui le icone si riferiscono.
        row.set("platformKnown", knownPlatformOf(streamUrl, platform));
        // Il profilo collegato rende disponibili anteprima, spot e i comandi di regia della riga.
        row.set("hasSetting", request.has("profileId") || request.has("profile"));
        const auto files = request.get("files"), scene = request.get("scene");
        const bool isScene = scene.isObject();
        const size_t total = isScene ? scene.get("items").size() : files.size();
        const double requestedPosition = row.get("position").num();
        const size_t position = total && std::isfinite(requestedPosition)
                                    ? size_t(std::clamp(requestedPosition, 0.0, double(total - 1)))
                                    : 0;
        row.set("position", double(position));
        row.set("total", double(total));
        row.set("isScene", isScene);
        row.set("isPlaylist", !isScene && files.size() > 1);
        const auto path = files.at(position).str();
        row.set("title", isScene ? scene.get("name").str() : fs::path(path).filename().string());
        // La playlist mostra la cartella da cui trasmette, non il singolo file in onda (come
        // FolderOfVideoToStream di Windows); la scena mostra la sua sorgente di base.
        std::string sceneSource = scene.get("name").str();
        const auto sceneItems = scene.get("items");
        for (size_t k = 0; k < sceneItems.size(); ++k) {
            const auto target = sceneItems.at(k).get("sourceTarget").str();
            if (!target.empty()) {
                sceneSource = fs::path(target).filename().string();
                break;
            }
        }
        row.set("source", isScene            ? sceneSource
                          : files.size() > 1 ? fs::path(path).parent_path().string()
                                             : path);
        if (current.has("takeover") && current.get("takeover").has("scene")) {
            const auto takeover = current.get("takeover");
            row.set("takeoverLabel", takeover.get("label").str());
            row.set("takeoverHolds", takeoverHolds(takeover));
        }
        rows.push_back(row);
    }
    std::sort(rows.begin(), rows.end(), [](const Json& a, const Json& b) {
        auto left = a.get("startedAt").str(), right = b.get("startedAt").str();
        return left != right ? left > right : a.get("id").num() > b.get("id").num();
    });
    auto result = Json::array();
    for (const auto& row : rows)
        result.push(row);
    return result;
}
} // namespace orbis
