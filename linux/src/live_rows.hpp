#pragma once
#include "core.hpp"

namespace orbis {
// Una riga per diretta/playlist, come MainLiveModel e _LiveRows.cshtml, anche dopo il riavvio
// dell'app.
Json liveRows(Store& store, const Json& sessions);
} // namespace orbis
