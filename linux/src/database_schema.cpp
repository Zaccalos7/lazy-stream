#include "database_schema.hpp"

#include <stdexcept>
#include <string>

namespace orbis {
namespace {
void execute(sqlite3* db, const char* sql) {
    char* error = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &error) != SQLITE_OK) {
        std::string message = error ? error : sqlite3_errmsg(db);
        sqlite3_free(error);
        throw std::runtime_error(message);
    }
}

int userVersion(sqlite3* db) {
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, "PRAGMA user_version", -1, &statement, nullptr) != SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db));
    const int result = sqlite3_step(statement);
    const int version = result == SQLITE_ROW ? sqlite3_column_int(statement, 0) : -1;
    sqlite3_finalize(statement);
    if (result != SQLITE_ROW)
        throw std::runtime_error("Versione schema SQLite non leggibile");
    return version;
}

int userTables(sqlite3* db) {
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db,
                           "SELECT count(*) FROM sqlite_master WHERE type='table' AND "
                           "name NOT LIKE 'sqlite_%'",
                           -1, &statement, nullptr) != SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db));
    const int result = sqlite3_step(statement);
    const int count = result == SQLITE_ROW ? sqlite3_column_int(statement, 0) : -1;
    sqlite3_finalize(statement);
    if (result != SQLITE_ROW)
        throw std::runtime_error("Schema SQLite non leggibile");
    return count;
}

bool hasRecords(sqlite3* db) {
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT 1 FROM sqlite_master WHERE type='table' AND name='records'",
                           -1, &statement, nullptr) != SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db));
    const int result = sqlite3_step(statement);
    sqlite3_finalize(statement);
    return result == SQLITE_ROW;
}
} // namespace

// Come DatabaseSchema.cs, PRAGMA user_version registra le migrazioni applicate una volta sola.
// Un database Windows o più nuovo viene rifiutato prima di creare tabelle Linux nello stesso file.
void ensureDatabaseSchema(sqlite3* db) {
    execute(db, "PRAGMA busy_timeout=5000");
    const int version = userVersion(db);
    if (version > databaseSchemaVersion)
        throw std::runtime_error("Database creato da una versione Linux più recente");
    if (version > 0 && !hasRecords(db))
        throw std::runtime_error("Schema database non compatibile con OrbisStream Linux");
    if (version == 0) {
        if (userTables(db) != 0)
            throw std::runtime_error("Database esistente senza schema Linux riconosciuto");
        execute(db, "BEGIN IMMEDIATE");
        try {
            execute(db, "CREATE TABLE records (kind TEXT NOT NULL,id INTEGER NOT NULL,data TEXT "
                        "NOT NULL,PRIMARY KEY(kind,id)); PRAGMA user_version=1");
            execute(db, "COMMIT");
        } catch (...) {
            execute(db, "ROLLBACK");
            throw;
        }
    }
    if (version < 2) {
        execute(db, "BEGIN IMMEDIATE");
        try {
            // Retire the removed provider without destroying user records or historical snapshots.
            // Domain recognition exists only in this one-time upgrade, never in the publisher.
            execute(db, R"sql(
                CREATE TABLE retired_records (
                    kind TEXT NOT NULL, id INTEGER NOT NULL, data TEXT NOT NULL,
                    reason TEXT NOT NULL, PRIMARY KEY(kind,id));
                WITH urls AS (
                    SELECT kind,id,data,lower(coalesce(json_extract(data,'$.streamUrl'),'')) AS url
                    FROM records WHERE kind IN ('channels','profiles')
                ), authorities AS (
                    SELECT *,substr(url,instr(url,'://')+3) AS authority FROM urls
                ), hosts AS (
                    SELECT *,substr(authority,1,instr(authority||'/','/')-1) AS hostport
                    FROM authorities
                ), normalized AS (
                    SELECT *,substr(hostport,1,instr(hostport||':',':')-1) AS host FROM hosts
                )
                INSERT INTO retired_records(kind,id,data,reason)
                SELECT kind,id,data,'provider-removed' FROM normalized
                WHERE lower(trim(coalesce(json_extract(data,'$.platform'),'')))='tiktok'
                   OR lower(trim(coalesce(json_extract(data,'$.platformStreamName'),'')))='tiktok'
                   OR lower(trim(coalesce(json_extract(data,'$.defaultPlatformConfiguration'),'')))='tiktok'
                   OR host IN ('tiktok.com','tiktokv.com','tiktokcdn.com','tiktokcdn-eu.com','tiktokcdn-us.com')
                   OR host LIKE '%.tiktok.com' OR host LIKE '%.tiktokv.com'
                   OR host LIKE '%.tiktokcdn.com' OR host LIKE '%.tiktokcdn-eu.com'
                   OR host LIKE '%.tiktokcdn-us.com';
                UPDATE records SET data=json_set(data,'$.profileId',0)
                WHERE kind='channels' AND json_extract(data,'$.profileId') IN
                    (SELECT id FROM retired_records WHERE kind='profiles');
                DELETE FROM records WHERE (kind,id) IN (SELECT kind,id FROM retired_records);
                PRAGMA user_version=2;
            )sql");
            execute(db, "COMMIT");
        } catch (...) {
            execute(db, "ROLLBACK");
            throw;
        }
    }
    execute(db, "PRAGMA journal_mode=WAL");
}
} // namespace orbis
