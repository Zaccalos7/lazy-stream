#pragma once

#include <sqlite3.h>

namespace orbis {
constexpr int databaseSchemaVersion = 2;
void ensureDatabaseSchema(sqlite3* db);
} // namespace orbis
