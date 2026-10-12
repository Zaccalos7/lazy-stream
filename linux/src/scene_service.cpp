#include "scene_service.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <set>

namespace orbis {
namespace {

// Una stringa di soli spazi non fornisce un nome o un percorso utilizzabile a FFmpeg.
bool blank(const std::string& value) {
    return std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
}

// I percorsi file incollati da un file manager possono avere spazi o virgolette ai bordi.
std::string userPath(std::string path) {
    auto trim = [](std::string& text) {
        auto first = text.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            text.clear();
            return;
        }
        text = text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
    };
    trim(path);
    if (path.size() >= 2 && path.front() == '"' && path.back() == '"')
        path = path.substr(1, path.size() - 2);
    trim(path);
    return path;
}

// Accetta i nomi wire e i vecchi valori numerici definiti da SourceKind.cs.
std::string sourceKind(const Json& item) {
    auto value = item.get("sourceKind").str("FILE");
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    if (value == "0")
        return "FILE";
    if (value == "1")
        return "SCREEN";
    if (value == "2")
        return "CAMERA";
    if (value == "3")
        return "MICROPHONE";
    if (value == "4")
        return "OVERLAY";
    if (value == "DISPLAY" || value == "DESKTOP")
        return "SCREEN";
    if (value == "WEBCAM" || value == "VIDEO")
        return "CAMERA";
    if (value == "MIC" || value == "AUDIO")
        return "MICROPHONE";
    if (value == "IMAGE")
        return "OVERLAY";
    if (value == "FILE" || value == "SCREEN" || value == "CAMERA" || value == "MICROPHONE" ||
        value == "OVERLAY")
        return value;
    throw std::runtime_error("Tipo sorgente non supportato");
}

// Confronta il percorso canonico con la libreria: un symlink verso l'esterno non è un overlay.
bool libraryOverlay(const fs::path& root, const std::string& target) {
    std::error_code ec;
    auto file = fs::canonical(target, ec);
    if (ec || !fs::is_regular_file(file))
        return false;
    auto library = fs::canonical(root / "overlays", ec);
    return !ec && file.parent_path() == library;
}

// Lo storico conserva uno snapshot della richiesta; il suo ID identifica la scena di partenza.
bool sceneOnAir(Store& store, long id) {
    auto history = store.list("history");
    for (size_t i = 0; i < history.size(); ++i)
        if (history.at(i).get("request").get("scene").get("id").num() == id)
            return true;
    return false;
}

// Controlla i rettangoli prima del salvataggio, quando un errore è ancora correggibile nell'editor.
void checkRectangle(const Json& item, double width, double height) {
    auto x = item.get("x").num(), y = item.get("y").num();
    auto w = item.get("width").num(), h = item.get("height").num();
    if (!std::isfinite(w) || !std::isfinite(h) || w <= 0 || h <= 0 || w > 7680 || h > 7680)
        throw std::runtime_error("La sorgente richiede larghezza e altezza positive");
    if (!std::isfinite(x) || !std::isfinite(y) || x < 0 || y < 0 || x + w > width || y + h > height)
        throw std::runtime_error("La sorgente esce dal canvas");
}

} // namespace

// Distingue gli slot di un layout dalle sorgenti di una scena, seguendo CheckSlots e CheckLayout.
void validateSceneForSave(const Json& scene, const fs::path& dataRoot) {
    if (!scene.isObject() || blank(scene.get("name").str()))
        throw std::runtime_error("Nome scena richiesto");
    const auto width = scene.get("width").num(), height = scene.get("height").num();
    if (!std::isfinite(width) || !std::isfinite(height) || width <= 0 || height <= 0 ||
        width > 7680 || height > 7680)
        throw std::runtime_error("Il canvas richiede larghezza e altezza positive");
    auto items = scene.get("items");
    if (!items.isArray() || items.size() == 0)
        throw std::runtime_error(scene.get("isLayout").flag() ? "Il layout richiede almeno uno slot"
                                                              : "La scena richiede una sorgente");
    if (items.size() > 32)
        throw std::runtime_error("Massimo 32 sorgenti");
    const bool layout = scene.get("isLayout").flag();
    bool picture = false;
    std::set<std::pair<std::string, std::string>> sources;
    for (size_t i = 0; i < items.size(); ++i) {
        auto item = items.at(i);
        if (!item.isObject())
            throw std::runtime_error("Sorgente JSON non valida");
        auto kind = sourceKind(item);
        auto target = kind == "FILE" ? userPath(item.get("sourceTarget").str())
                                     : item.get("sourceTarget").str();
        if (kind == "OVERLAY" && !libraryOverlay(dataRoot, target))
            throw std::runtime_error("Overlay non presente nella libreria");
        if (layout) {
            checkRectangle(item, width, height);
            continue;
        }
        if (blank(target))
            throw std::runtime_error("La sorgente richiede un percorso o dispositivo");
        if (kind == "FILE" && !fs::is_regular_file(target))
            throw std::runtime_error("File della scena non trovato");
        if (kind != "OVERLAY" && !sources.emplace(kind, target).second)
            throw std::runtime_error("Sorgente duplicata nella scena");
        if (kind != "MICROPHONE") {
            picture = true;
            checkRectangle(item, width, height);
        }
    }
    if (!layout && !picture)
        throw std::runtime_error("La scena richiede almeno una sorgente video");
}

// La scena usata da una diretta resta immutabile; la modifica riceve un nuovo ID come su Windows.
long saveScene(Store& store, Json scene, long id) {
    validateSceneForSave(scene, store.root);
    if (id > 0) {
        auto old = store.get("scenes", id);
        if (old.get("isLayout").flag() != scene.get("isLayout").flag() ||
            (!old.get("isLayout").flag() && sceneOnAir(store, id)))
            id = 0;
    }
    auto items = scene.get("items");
    for (size_t i = 0; i < items.size(); ++i) {
        auto item = items.at(i);
        auto kind = sourceKind(item);
        item.set("sourceKind", kind);
        if (kind == "FILE")
            item.set("sourceTarget", userPath(item.get("sourceTarget").str()));
        if (kind == "OVERLAY")
            item.set("sourceTarget", fs::canonical(item.get("sourceTarget").str()).string());
        if (scene.get("isLayout").flag() && kind != "OVERLAY") {
            item.set("sourceTarget", "");
            item.set("audioEnabled", false);
        }
    }
    return store.save("scenes", scene, id);
}

// Una scena storica non può essere eliminata senza rompere il riavvio della sua diretta.
void ensureSceneCanDelete(Store& store, long id) {
    auto scene = store.get("scenes", id);
    if (!scene.get("isLayout").flag() && sceneOnAir(store, id))
        throw std::runtime_error("La scena è usata dallo storico delle dirette");
}

// Include gli snapshot dello storico, perché una diretta deve poter riaprire il suo overlay.
bool overlayInUse(Store& store, const fs::path& file) {
    auto usedBy = [&](const Json& scene) {
        auto items = scene.get("items");
        for (size_t i = 0; i < items.size(); ++i)
            if (sourceKind(items.at(i)) == "OVERLAY" &&
                items.at(i).get("sourceTarget").str() == file.string())
                return true;
        return false;
    };
    for (auto list : {store.list("scenes"), store.list("history")})
        for (size_t i = 0; i < list.size(); ++i)
            if (usedBy(list.at(i).has("request") ? list.at(i).get("request").get("scene")
                                                 : list.at(i)))
                return true;
    return false;
}

} // namespace orbis
