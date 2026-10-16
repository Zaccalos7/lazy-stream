#include "core.hpp"
#include "database_schema.hpp"
#include "history_cleanup_service.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <poll.h>
#include <regex>
#include <signal.h>
#include <spawn.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;

namespace orbis {
Json::Json() : Json(json_object_new_object()) {}

Json::Json(json_object* value, bool retain)
    : p(retain && value ? json_object_get(value) : value, json_object_put) {}

// Legge un solo documento JSON; rifiuta testo residuo per non accettare richieste ambigue.
Json Json::parse(const std::string& text) {
    json_tokener* tok = json_tokener_new();
    auto* obj = json_tokener_parse_ex(tok, text.c_str(), static_cast<int>(text.size()));
    bool ok = json_tokener_get_error(tok) == json_tokener_success;
    size_t consumed = json_tokener_get_parse_end(tok);
    json_tokener_free(tok);
    if (!ok || text.find_first_not_of(" \n\r\t", consumed) != std::string::npos) {
        if (obj)
            json_object_put(obj);
        throw std::runtime_error("JSON non valido");
    }
    return Json(obj);
}

Json Json::array() {
    return Json(json_object_new_array());
}

Json Json::string(const std::string& s) {
    return Json(json_object_new_string_len(s.c_str(), s.size()));
}

// Mantiene interi i valori senza decimali, così gli identificativi restano compatibili con le
// route.
Json Json::number(double n) {
    return std::isfinite(n) && n == std::floor(n) && std::abs(n) < 9e15
               ? Json(json_object_new_int64(static_cast<int64_t>(n)))
               : Json(json_object_new_double(n));
}

Json Json::boolean(bool b) {
    return Json(json_object_new_boolean(b));
}

// Restituisce una vista con riferimento mantenuto: evita copie ma condivide le modifiche al nodo.
Json Json::get(const std::string& key) const {
    json_object* v = nullptr;
    if (p)
        json_object_object_get_ex(p.get(), key.c_str(), &v);
    return Json(v, true);
}

// Mantiene vivo il nodo dell’array senza copiarlo; usare clone() quando serve uno snapshot
// indipendente.
Json Json::at(size_t index) const {
    return Json(isArray() ? json_object_array_get_idx(p.get(), index) : nullptr, true);
}

void Json::set(const std::string& k, const Json& v) {
    json_object_object_add(p.get(), k.c_str(), v.p ? json_object_get(v.p.get()) : nullptr);
}

void Json::set(const std::string& k, const std::string& v) {
    set(k, string(v));
}

void Json::set(const std::string& k, double v) {
    set(k, number(v));
}

void Json::set(const std::string& k, bool v) {
    set(k, boolean(v));
}

void Json::push(const Json& v) {
    json_object_array_add(p.get(), v.p ? json_object_get(v.p.get()) : nullptr);
}

bool Json::has(const std::string& k) const {
    json_object* v = nullptr;
    return p && json_object_object_get_ex(p.get(), k.c_str(), &v);
}

bool Json::isArray() const {
    return p && json_object_get_type(p.get()) == json_type_array;
}

bool Json::isObject() const {
    return p && json_object_get_type(p.get()) == json_type_object;
}

std::string Json::str(const std::string& fallback) const {
    return p ? json_object_get_string(p.get()) : fallback;
}

double Json::num(double fallback) const {
    return p ? json_object_get_double(p.get()) : fallback;
}

bool Json::flag(bool fallback) const {
    return p ? json_object_get_boolean(p.get()) : fallback;
}

size_t Json::size() const {
    return isArray() ? json_object_array_length(p.get()) : 0;
}

std::string Json::dump() const {
    return p ? json_object_to_json_string_ext(p.get(), JSON_C_TO_STRING_PLAIN) : "null";
}

// Uniforma le risposte positive, così il client può mostrare sempre lo stesso campo di messaggio.
Json success(const std::string& m) {
    Json j;
    j.set("response", "success");
    j.set("message", m);
    return j;
}

// Legge il file in modalità binaria per preservare anche immagini e contenuti non testuali.
std::string readFile(const fs::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f)
        throw std::runtime_error("File non leggibile: " + path.string());
    return {std::istreambuf_iterator<char>(f), {}};
}

// Scrive un temporaneo privato e lo rinomina: i lettori non vedono un file aggiornato a metà.
void writeFile(const fs::path& path, const std::string& data) {
    fs::create_directories(path.parent_path());
    auto tmp = path.string() + ".tmp";
    std::ofstream f(tmp, std::ios::binary);
    f.write(data.data(), data.size());
    f.close();
    if (!f)
        throw std::runtime_error("Scrittura non riuscita");
    chmod(tmp.c_str(), 0600);
    fs::rename(tmp, path);
}

// Produce date UTC confrontabili e indipendenti dal fuso orario della macchina.
std::string now() {
    auto t = std::time(nullptr);
    std::tm tm{};
    gmtime_r(&t, &tm);
    std::ostringstream s;
    s << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return s.str();
}

// Confronta le sequenze numeriche per lunghezza e cifre: ordina video2 prima di video10 senza
// overflow.
bool naturalLess(const std::string& a, const std::string& b) {
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (std::isdigit(static_cast<unsigned char>(a[i])) &&
            std::isdigit(static_cast<unsigned char>(b[j]))) {
            size_t x = i, y = j;
            while (i < a.size() && std::isdigit(static_cast<unsigned char>(a[i])))
                ++i;
            while (j < b.size() && std::isdigit(static_cast<unsigned char>(b[j])))
                ++j;
            auto aa = a.substr(x, i - x), bb = b.substr(y, j - y);
            auto ax = aa.find_first_not_of('0'), bx = bb.find_first_not_of('0');
            aa = ax == std::string::npos ? "0" : aa.substr(ax);
            bb = bx == std::string::npos ? "0" : bb.substr(bx);
            if (aa.size() != bb.size())
                return aa.size() < bb.size();
            if (aa != bb)
                return aa < bb;
        } else {
            auto x = std::tolower(static_cast<unsigned char>(a[i++])),
                 y = std::tolower(static_cast<unsigned char>(b[j++]));
            if (x != y)
                return x < y;
        }
    }
    return a.size() < b.size();
}

namespace {
struct Statement {
    sqlite3_stmt* p = nullptr;

    Statement(sqlite3* db, const char* sql) {
        if (sqlite3_prepare_v2(db, sql, -1, &p, nullptr) != SQLITE_OK)
            throw std::runtime_error(sqlite3_errmsg(db));
    }

    ~Statement() {
        sqlite3_finalize(p);
    }

    void text(int n, const std::string& s) {
        sqlite3_bind_text(p, n, s.c_str(), -1, SQLITE_TRANSIENT);
    }

    void integer(int n, long v) {
        sqlite3_bind_int64(p, n, v);
    }

    int step() {
        int r = sqlite3_step(p);
        if (r != SQLITE_ROW && r != SQLITE_DONE)
            throw std::runtime_error(sqlite3_errmsg(sqlite3_db_handle(p)));
        return r;
    }
};

// Esegue istruzioni SQL senza risultati e converte gli errori SQLite in eccezioni del servizio.
void exec(sqlite3* db, const char* sql) {
    char* error = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &error) != SQLITE_OK) {
        std::string s = error;
        sqlite3_free(error);
        throw std::runtime_error(s);
    }
}

// Limita le categorie persistibili a quelle previste, evitando record fuori dal modello
// applicativo.
bool validKind(const std::string& k) {
    return k == "channels" || k == "profiles" || k == "scenes" || k == "buttons" || k == "videos" ||
           k == "history" || k == "preferences";
}
} // namespace

// Prepara SQLite e i dati iniziali; recupera le dirette rimaste aperte dopo un arresto anomalo.
Store::Store(fs::path path) : root(std::move(path)) {
    fs::create_directories(root);
    chmod(root.c_str(), 0700);
    for (auto s : {"logs", "overlays", "scene-media", "previews", "images"})
        fs::create_directories(root / s);
    if (sqlite3_open((root / "stream.db").c_str(), &db) != SQLITE_OK)
        throw std::runtime_error("Database non disponibile");
    chmod((root / "stream.db").c_str(), 0600);
    try {
        ensureDatabaseSchema(db);
    } catch (...) {
        sqlite3_close(db);
        db = nullptr;
        throw;
    }
    if (list("profiles").size() == 0) {
        for (auto platform : {"Custom", "YouTube", "Twitch", "Kick", "Facebook", "TikTok"}) {
            Json p;
            p.set("title", std::string("Default ") + platform);
            p.set("platform", platform);
            p.set("isDefaultConfiguration", true);
            p.set("isActive", false);
            p.set("videoCodecName", "libx264");
            p.set("videoBitrate", 4500.0);
            p.set("videoWidth", platform == std::string("TikTok") ? 720.0 : 1280.0);
            p.set("videoHeight", platform == std::string("TikTok") ? 1280.0 : 720.0);
            p.set("frameRate", 30.0);
            p.set("gopSize", 2.0);
            p.set("preset", "veryfast");
            p.set("audioBitrate", 128.0);
            p.set("lowLatency", true);
            save("profiles", p);
        }
    }
    auto history = list("history");
    for (size_t i = 0; i < history.size(); ++i) {
        auto h = history.at(i);
        if (h.get("status").str() == "LIVE" || h.get("status").str() == "STARTING" ||
            h.get("status").str() == "RECONNECTING") {
            h.set("status", "INTERRUPTED");
            h.set("endedAt", now());
            save("history", h, h.get("id").num());
        }
    }
}

// Chiude la connessione quando lo store esce di vita, liberando le risorse SQLite.
Store::~Store() {
    sqlite3_close(db);
}

// Legge una categoria sotto mutex, così le richieste concorrenti condividono la connessione in
// sicurezza.
Json Store::list(const std::string& k) {
    std::lock_guard lock(mutex);
    Statement q(db, "SELECT data FROM records WHERE kind=? ORDER BY id");
    q.text(1, k);
    auto a = Json::array();
    while (q.step() == SQLITE_ROW)
        a.push(Json::parse(reinterpret_cast<const char*>(sqlite3_column_text(q.p, 0))));
    return a;
}

// Cerca un record per categoria e ID con parametri SQL, separando i dati dalla query.
Json Store::get(const std::string& k, long id) {
    std::lock_guard lock(mutex);
    Statement q(db, "SELECT data FROM records WHERE kind=? AND id=?");
    q.text(1, k);
    q.integer(2, id);
    if (q.step() != SQLITE_ROW)
        throw std::runtime_error("Elemento non trovato");
    return Json::parse(reinterpret_cast<const char*>(sqlite3_column_text(q.p, 0)));
}

// Valida e inserisce o aggiorna un record sotto lock, mantenendo coerente l’assegnazione degli ID.
long Store::save(const std::string& k, Json v, long id) {
    std::lock_guard lock(mutex);
    if (!validKind(k))
        throw std::runtime_error("Tipo non valido");
    if (!v.isObject())
        throw std::runtime_error("Oggetto JSON richiesto");
    if (id <= 0) {
        Statement q(db, "SELECT coalesce(max(id),0)+1 FROM records WHERE kind=?");
        q.text(1, k);
        q.step();
        id = sqlite3_column_int64(q.p, 0);
    }
    v.set("id", double(id));
    v.set("pkid", double(id));
    v.set("lastModified", now());
    Statement q(
        db,
        "INSERT INTO records VALUES(?,?,?) ON CONFLICT(kind,id) DO UPDATE SET data=excluded.data");
    q.text(1, k);
    q.integer(2, id);
    q.text(3, v.dump());
    q.step();
    return id;
}

// Elimina solo il record richiesto; i parametri SQL evitano di costruire query dai valori del
// client.
void Store::remove(const std::string& k, long id) {
    std::lock_guard lock(mutex);
    Statement q(db, "DELETE FROM records WHERE kind=? AND id=?");
    q.text(1, k);
    q.integer(2, id);
    q.step();
}

// Esporta una vista coerente delle impostazioni ed esclude le chiavi per rendere condivisibile il
// backup.
Json Store::exportData() {
    std::lock_guard lock(mutex);
    Json data;
    data.set("version", 1.0);
    for (auto k :
         {"channels", "profiles", "scenes", "buttons", "videos", "history", "preferences"}) {
        auto a = list(k);
        for (size_t i = 0; i < a.size(); ++i) {
            auto v = a.at(i);
            v.set("streamKey", "");
            if (v.has("request")) {
                auto r = v.get("request");
                r.set("streamKey", "");
                v.set("request", r);
            }
        }
        data.set(k, a);
    }
    return data;
}

// Sostituisce i dati in transazione: se un record fallisce, il rollback conserva la configurazione
// precedente.
void Store::importData(const Json& data) {
    std::lock_guard lock(mutex);
    if (data.get("version").num() != 1)
        throw std::runtime_error("Versione export non supportata");
    exec(db, "BEGIN IMMEDIATE");
    try {
        for (auto k : {"channels", "profiles", "scenes", "buttons", "videos", "preferences"}) {
            auto a = data.get(k);
            if (!a.isArray())
                throw std::runtime_error("Elenco mancante");
            std::vector<long> ids;
            for (size_t i = 0; i < a.size(); ++i) {
                auto v = a.at(i);
                long id = v.get("id").num();
                if (id <= 0 || std::find(ids.begin(), ids.end(), id) != ids.end())
                    throw std::runtime_error("ID duplicato o invalido");
                ids.push_back(id);
                validate(k, v);
            }
            Statement q(db, "DELETE FROM records WHERE kind=?");
            q.text(1, k);
            q.step();
            for (size_t i = 0; i < a.size(); ++i)
                save(k, a.at(i), a.at(i).get("id").num());
        }
        exec(db, "COMMIT");
    } catch (...) {
        exec(db, "ROLLBACK");
        throw;
    }
}

// Ricontrolla stato e identità sotto il mutex SQLite: un elenco precedente non basta a proteggere
// una live.
bool Store::removeHistoryIfInactive(long id, const std::string& startedAt) {
    std::lock_guard lock(mutex);
    Statement q(db, "SELECT data FROM records WHERE kind='history' AND id=?");
    q.integer(1, id);
    if (q.step() != SQLITE_ROW)
        return false;
    auto item = Json::parse(reinterpret_cast<const char*>(sqlite3_column_text(q.p, 0)));
    auto status = item.get("status").str();
    if (status == "LIVE" || status == "STARTING" || status == "RECONNECTING" ||
        item.get("startedAt").str() != startedAt)
        return false;
    remove("history", id);
    return true;
}

// Mantiene l'API sincrona precedente ma usa le stesse soglie e protezioni del servizio progressivo.
int Store::cleanup(int months, long channel) {
    auto cutoff = historyCleanupThreshold(months);
    int n = 0;
    auto a = list("history");
    for (size_t i = 0; i < a.size(); ++i) {
        auto v = a.at(i);
        auto status = v.get("status").str();
        if (status != "LIVE" && status != "STARTING" && status != "RECONNECTING" &&
            !v.get("startedAt").str().empty() && v.get("startedAt").str() < cutoff &&
            (!channel || v.get("channelId").num() == channel) &&
            removeHistoryIfInactive(v.get("id").num(), v.get("startedAt").str())) {
            ++n;
        }
    }
    return n;
}

// Controlla i vincoli prima della persistenza, così dati non validi non raggiungono il motore
// streaming.
void validate(const std::string& k, const Json& v) {
    if (!v.isObject())
        throw std::runtime_error("Oggetto JSON richiesto");
    if (k == "channels" &&
        (v.get("channelName").str().empty() || (!v.get("streamUrl").str().starts_with("rtmp://") &&
                                                !v.get("streamUrl").str().starts_with("rtmps://"))))
        throw std::runtime_error("Nome canale e server RTMP/RTMPS richiesti");
    if (k == "profiles") {
        if (v.get("title").str().empty())
            throw std::runtime_error("Titolo richiesto");
        for (auto field : {"videoWidth", "videoHeight"}) {
            int n = v.get(field).num(1280);
            if (n < 16 || n > 7680 || n % 2)
                throw std::runtime_error("Risoluzioni pari tra 16 e 7680");
        }
        if (v.get("frameRate").num(30) < 1 || v.get("frameRate").num(30) > 120 ||
            v.get("videoBitrate").num(4500) < 100 || v.get("videoBitrate").num(4500) > 100000)
            throw std::runtime_error("FPS o bitrate non validi");
    }
    if (k == "scenes") {
        if (v.get("name").str().empty())
            throw std::runtime_error("Nome scena richiesto");
        if (v.get("items").size() > 32)
            throw std::runtime_error("Massimo 32 sorgenti");
    }
    if (k == "buttons" && (v.get("position").num() < 0 || v.get("position").num() > 5 ||
                           v.get("label").str().empty()))
        throw std::runtime_error("Pulsante: posizione 0–5 e nome richiesti");
    if (k == "buttons" && !v.get("hotkey").str().empty()) {
        auto key = v.get("hotkey").str();
        const std::regex pattern(
            "^(Ctrl\\+)?(Alt\\+)?(Shift\\+)?(Key[A-Z]|Digit[0-9]|Numpad[0-9]|Numpad(Add|Subtract|"
            "Multiply|Divide|Decimal)|F([1-9]|1[0-9]|2[0-4])|Backquote|Minus|Equal|BracketLeft|"
            "BracketRight|Backslash|IntlBackslash|Semicolon|Quote|Comma|Period|Slash|Insert|Delete|"
            "Home|End|PageUp|PageDown|Arrow(Up|Down|Left|Right))$");
        const std::vector<std::string> reserved = {"F5",
                                                   "F11",
                                                   "F12",
                                                   "Ctrl+KeyR",
                                                   "Ctrl+KeyW",
                                                   "Ctrl+KeyF",
                                                   "Ctrl+KeyP",
                                                   "Ctrl+KeyN",
                                                   "Ctrl+KeyT",
                                                   "Ctrl+Shift+KeyI",
                                                   "Ctrl+Shift+KeyJ",
                                                   "Ctrl+F5",
                                                   "Shift+F5",
                                                   "Alt+F4"};
        if (!std::regex_match(key, pattern) ||
            std::find(reserved.begin(), reserved.end(), key) != reserved.end())
            throw std::runtime_error("Scorciatoia non valida o riservata al browser");
    }
}

// Arresta il processo posseduto: il ciclo di vita della risorsa segue quello dell’oggetto C++.
Process::~Process() {
    stop();
}

// Avvia argomenti separati tramite helper, senza shell; gruppo e pipe consentono controllo e
// lettura dell’output.
void Process::start(const std::vector<std::string>& args, int input, const fs::path& log,
                    bool pipeOutput, std::function<std::string(const std::string&)> redact) {
    if (args.empty())
        throw std::runtime_error("Comando vuoto");
    stop();
    // Un aggiornamento può sostituire il binario mentre l'app resta aperta: il link
    // /proc conserva la directory anche quando il nome termina con " (deleted)".
    auto runner = fs::read_symlink("/proc/self/exe").parent_path() / "orbis-stream-runner";
    int fds[2]{-1, -1};
    if (pipeOutput && pipe2(fds, O_CLOEXEC))
        throw std::runtime_error("pipe fallita");
    int errors[2]{-1, -1};
    if (redact && pipe2(errors, O_CLOEXEC)) {
        for (auto fd : fds)
            if (fd >= 0)
                close(fd);
        throw std::runtime_error("Pipe diagnostica fallita");
    }
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    if (input >= 0)
        posix_spawn_file_actions_adddup2(&actions, input, STDIN_FILENO);
    else
        posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
    if (pipeOutput) {
        posix_spawn_file_actions_adddup2(&actions, fds[1], STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&actions, fds[0]);
        posix_spawn_file_actions_addclose(&actions, fds[1]);
    } else
        posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0);
    if (redact) {
        posix_spawn_file_actions_adddup2(&actions, errors[1], STDERR_FILENO);
        posix_spawn_file_actions_addclose(&actions, errors[0]);
        posix_spawn_file_actions_addclose(&actions, errors[1]);
    } else
        posix_spawn_file_actions_addopen(&actions, STDERR_FILENO,
                                         log.empty() ? "/dev/null" : log.c_str(),
                                         O_WRONLY | O_CREAT | O_APPEND, 0600);
    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETSIGMASK);
    posix_spawnattr_setpgroup(&attr, 0);
    sigset_t empty;
    sigemptyset(&empty);
    posix_spawnattr_setsigmask(&attr, &empty);
    std::string executable = runner.string(), parent = std::to_string(getpid());
    std::vector<char*> argv = {executable.data(), parent.data()};
    for (auto& a : args)
        argv.push_back(const_cast<char*>(a.c_str()));
    argv.push_back(nullptr);
    pid_t child;
    int result = posix_spawnp(&child, executable.c_str(), &actions, &attr, argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);
    posix_spawnattr_destroy(&attr);
    if (fds[1] >= 0)
        close(fds[1]);
    if (errors[1] >= 0)
        close(errors[1]);
    if (result) {
        if (fds[0] >= 0)
            close(fds[0]);
        if (errors[0] >= 0)
            close(errors[0]);
        throw std::runtime_error("Impossibile avviare " + args[0] +
                                 "; verifica orbis-stream-runner");
    }
    pid = child;
    output = fds[0];
    exitCode = 0;
    if (redact)
        logReader = std::thread([fd = errors[0], log, redact] {
            std::ofstream file(log, std::ios::app);
            std::error_code permissionError;
            fs::permissions(log, fs::perms::owner_read | fs::perms::owner_write, permissionError);
            std::string pending;
            char buffer[4096];
            for (;;) {
                auto count = read(fd, buffer, sizeof(buffer));
                if (count < 0 && errno == EINTR)
                    continue;
                if (count <= 0)
                    break;
                pending.append(buffer, count);
                size_t end;
                while ((end = pending.find_first_of("\r\n")) != std::string::npos) {
                    file << redact(pending.substr(0, end)) << '\n';
                    pending.erase(0, end + 1);
                }
                // Bound malformed diagnostic lines without writing a partial secret.
                if (pending.size() > 1024 * 1024)
                    pending = "[diagnostic line exceeded limit]";
                file.flush();
            }
            if (!pending.empty())
                file << redact(pending) << '\n';
            close(fd);
        });
}

// Controlla il figlio senza bloccare, permettendo al ciclo streaming di rilevare un’uscita
// inattesa.
bool Process::running() {
    if (pid < 0)
        return false;
    int status;
    auto n = waitpid(pid, &status, WNOHANG);
    if (n == pid) {
        exitCode = WIFEXITED(status)     ? WEXITSTATUS(status)
                   : WIFSIGNALED(status) ? 128 + WTERMSIG(status)
                                         : 128;
        pid = -1;
        return false;
    }
    return n == 0;
}

// Attende la conclusione e restituisce l’esito per distinguere un risultato completo da un errore.
int Process::wait() {
    if (pid < 0)
        return exitCode;
    int status = 0;
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
    }
    pid = -1;
    exitCode = WIFEXITED(status)     ? WEXITSTATUS(status)
               : WIFSIGNALED(status) ? 128 + WTERMSIG(status)
                                     : 128;
    return exitCode;
}

// Termina il gruppo prima con SIGTERM e poi con SIGKILL, evitando figli bloccati durante la
// chiusura.
void Process::stop() {
    if (pid > 0) {
        kill(-pid, SIGTERM);
        for (int i = 0; i < 30 && running(); ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        if (pid > 0) {
            kill(-pid, SIGKILL);
            wait();
        }
    }
    if (output >= 0) {
        close(output);
        output = -1;
    }
    if (logReader.joinable())
        logReader.join();
}

// Raccoglie stdout con timeout e limite di dimensione, così un comando esterno non blocca o satura
// il server.
std::string capture(const std::vector<std::string>& args, int timeout) {
    Process p;
    p.start(args);
    std::string out;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout);
    char buf[8192];
    while (std::chrono::steady_clock::now() < deadline) {
        pollfd fd{p.fd(), POLLIN, 0};
        if (poll(&fd, 1, 100) > 0) {
            auto n = read(p.fd(), buf, sizeof(buf));
            if (n <= 0) {
                if (p.wait() != 0)
                    throw std::runtime_error("Comando fallito: " + args[0]);
                return out;
            }
            out.append(buf, n);
            if (out.size() > 16 * 1024 * 1024)
                throw std::runtime_error("Output troppo grande");
        }
    }
    throw std::runtime_error("Tempo scaduto: " + args[0]);
}

// Consente un eseguibile FFmpeg configurato via ambiente senza fissare un percorso specifico della
// distribuzione.
std::string ffmpeg() {
    auto p = std::getenv("FFMPEG_PATH");
    return p ? p : "ffmpeg";
}

// Consente un FFprobe configurato via ambiente per usare la stessa installazione scelta
// dall’utente.
std::string ffprobe() {
    auto p = std::getenv("FFPROBE_PATH");
    return p ? p : "ffprobe";
}

// Interroga FFprobe in JSON per ottenere proprietà reali del media anziché dedurle dall’estensione.
Json probe(const std::string& f) {
    if (!fs::is_regular_file(f))
        throw std::runtime_error("File media inesistente");
    return Json::parse(
        capture({ffprobe(), "-v", "error", "-show_streams", "-show_format", "-of", "json", f}));
}

// Raccoglie i video locali e applica l’ordine naturale, rendendo prevedibile la playlist importata.
std::vector<std::string> scanVideos(const fs::path& dir) {
    std::vector<std::string> out;
    const std::regex ext("\\.(mp4|mkv|mov|webm|avi|m4v|ts|flv|mpeg|mpg)$", std::regex::icase);
    for (auto& e : fs::directory_iterator(dir))
        if (e.is_regular_file() && std::regex_search(e.path().string(), ext))
            out.push_back(fs::absolute(e.path()).string());
    std::sort(out.begin(), out.end(), naturalLess);
    return out;
}

// Elenca sorgenti X11, V4L2 e PulseAudio disponibili: il browser può scegliere dispositivi del
// computer host.
Json sources() {
    auto out = Json::array();
    if (const auto* display = std::getenv("DISPLAY")) {
        Json v;
        v.set("kind", "SCREEN");
        v.set("target", display);
        v.set("label", "Desktop X11");
        out.push(v);
    }
    for (auto& e : fs::directory_iterator("/dev")) {
        if (e.path().filename().string().starts_with("video")) {
            Json v;
            v.set("kind", "CAMERA");
            v.set("target", e.path().string());
            auto name = fs::path("/sys/class/video4linux") / e.path().filename() / "name";
            v.set("label", fs::exists(name) ? readFile(name) : e.path().filename().string());
            out.push(v);
        }
    }
    try {
        auto lines = capture({"pactl", "list", "short", "sources"}, 3);
        std::istringstream s(lines);
        std::string line;
        while (std::getline(s, line)) {
            std::istringstream l(line);
            std::string id, name;
            l >> id >> name;
            Json v;
            v.set("kind", "MICROPHONE");
            v.set("target", name);
            v.set("label", name);
            out.push(v);
        }
    } catch (...) {
    }
    return out;
}

// Campiona contatori Linux e spazio disco; usa differenze temporali per ricavare velocità e
// utilizzo.
Json metrics(const fs::path& root) {
    static std::mutex mutex;
    std::lock_guard lock(mutex);
    static unsigned long long lastTotal = 0, lastIdle = 0, lastRx = 0, lastTx = 0;
    static auto lastTime = std::chrono::steady_clock::now();
    Json out;
    std::ifstream f("/proc/stat");
    std::string line;
    std::getline(f, line);
    std::istringstream s(line);
    std::string cpu;
    s >> cpu;
    unsigned long long total = 0, idle = 0, n = 0;
    for (int i = 0; s >> n; ++i) {
        if (i < 8)
            total += n;
        if (i == 3 || i == 4)
            idle += n;
    }
    out.set("cpu", lastTotal && total > lastTotal
                       ? 100.0 * (1.0 - double(idle - lastIdle) / double(total - lastTotal))
                       : -1.0);
    lastTotal = total;
    lastIdle = idle;
    std::ifstream mem("/proc/meminfo");
    std::map<std::string, double> vals;
    while (std::getline(mem, line)) {
        std::istringstream l(line);
        std::string key;
        double value;
        l >> key >> value;
        vals[key] = value;
    }
    out.set("ram",
            vals["MemTotal:"] ? 100.0 * (1 - vals["MemAvailable:"] / vals["MemTotal:"]) : -1.0);
    out.set("swap",
            vals["SwapTotal:"] ? 100.0 * (1 - vals["SwapFree:"] / vals["SwapTotal:"]) : 0.0);
    out.set("memoryBytes", vals["MemTotal:"] * 1024);
    std::ifstream net("/proc/net/dev");
    unsigned long long rx = 0, tx = 0;
    while (std::getline(net, line)) {
        auto pos = line.find(':');
        if (pos == std::string::npos ||
            (line.substr(0, pos).find_first_not_of(" \t") != std::string::npos &&
             line.substr(line.find_first_not_of(" \t"), pos - line.find_first_not_of(" \t")) ==
                 "lo"))
            continue;
        std::istringstream l(line.substr(pos + 1));
        for (int i = 0; i < 16 && l >> n; ++i) {
            if (i == 0)
                rx += n;
            if (i == 8)
                tx += n;
        }
    }
    auto current = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(current - lastTime).count();
    out.set("networkRx", lastRx && rx >= lastRx ? double(rx - lastRx) / elapsed : -1);
    out.set("networkTx", lastTx && tx >= lastTx ? double(tx - lastTx) / elapsed : -1);
    lastRx = rx;
    lastTx = tx;
    lastTime = current;
    struct statvfs disk{};
    if (!statvfs(root.c_str(), &disk)) {
        out.set("diskAvailable", double(disk.f_bavail) * disk.f_frsize);
        out.set("diskTotal", double(disk.f_blocks) * disk.f_frsize);
    }
    std::ifstream up("/proc/uptime");
    double uptime = 0;
    up >> uptime;
    out.set("uptime", uptime);
    auto temps = Json::array();
    if (fs::exists("/sys/class/thermal"))
        for (auto& e : fs::directory_iterator("/sys/class/thermal"))
            if (fs::exists(e.path() / "temp")) {
                try {
                    Json t;
                    t.set("label", readFile(e.path() / "type"));
                    t.set("celsius", std::stod(readFile(e.path() / "temp")) / 1000);
                    temps.push(t);
                } catch (...) {
                }
            }
    out.set("temperatures", temps);
    double cpuTemp = -1;
    for (size_t i = 0; i < temps.size(); ++i) {
        auto t = temps.at(i);
        auto label = t.get("label").str();
        if (label.find("cpu") != std::string::npos || label.find("x86_pkg") != std::string::npos)
            cpuTemp = std::max(cpuTemp, t.get("celsius").num());
    }
    out.set("cpu_temperature", cpuTemp);
    out.set("gpu_temperature", -1.0);
    static auto gpuTime = std::chrono::steady_clock::time_point{};
    static Json gpu;
    auto stamp = std::chrono::steady_clock::now();
    if (stamp - gpuTime > std::chrono::seconds(30)) {
        gpuTime = stamp;
        try {
            auto csv =
                capture({"nvidia-smi",
                         "--query-gpu=temperature.gpu,utilization.gpu,memory.used,memory.total",
                         "--format=csv,noheader,nounits"},
                        3);
            std::replace(csv.begin(), csv.end(), ',', ' ');
            std::istringstream in(csv);
            double temp, use, used, total;
            if (in >> temp >> use >> used >> total) {
                gpu.set("temperature", temp);
                gpu.set("utilization", use);
                gpu.set("memoryUsed", used * 1024 * 1024);
                gpu.set("memoryTotal", total * 1024 * 1024);
            }
        } catch (...) {
        }
    }
    out.set("gpu", gpu);
    if (gpu.has("temperature"))
        out.set("gpu_temperature", gpu.get("temperature"));
    static unsigned long long lastRead = 0, lastWrite = 0;
    unsigned long long readBytes = 0, writeBytes = 0;
    std::ifstream disks("/proc/diskstats");
    while (std::getline(disks, line)) {
        std::istringstream l(line);
        int major, minor;
        std::string dev;
        l >> major >> minor >> dev;
        if (!fs::exists(fs::path("/sys/block") / dev) || dev.starts_with("loop") ||
            dev.starts_with("ram") || dev.starts_with("dm-"))
            continue;
        for (int i = 0; i < 11 && l >> n; ++i) {
            if (i == 2)
                readBytes += n * 512;
            if (i == 6)
                writeBytes += n * 512;
        }
    }
    out.set("diskRead",
            lastRead && readBytes >= lastRead ? double(readBytes - lastRead) / elapsed : -1);
    out.set("diskWrite",
            lastWrite && writeBytes >= lastWrite ? double(writeBytes - lastWrite) / elapsed : -1);
    lastRead = readBytes;
    lastWrite = writeBytes;
    out.set("app", processMetrics(getpid()));
    out.set("threads", double(std::thread::hardware_concurrency()));
    return out;
}
} // namespace orbis
