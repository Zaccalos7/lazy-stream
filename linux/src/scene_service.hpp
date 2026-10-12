#pragma once
#include "core.hpp"

namespace orbis {

// Applica i controlli di SceneService.cs usando la libreria overlay nella cartella dati Linux.
void validateSceneForSave(const Json& scene, const fs::path& dataRoot);

// Salva una nuova scena se cambiano tipo o contenuto di una scena già usata in una diretta.
long saveScene(Store& store, Json scene, long id = 0);

// Impedisce di cancellare una scena che lo storico di una diretta deve poter riaprire.
void ensureSceneCanDelete(Store& store, long id);

// Protegge gli overlay ancora nominati da scene o snapshot delle dirette.
bool overlayInUse(Store& store, const fs::path& file);

} // namespace orbis
