#pragma once
#include "change_notifier.hpp"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <json-c/json.h>
#include <map>
#include <memory>
#include <mutex>
#include <sqlite3.h>
#include <string>
#include <thread>
#include <vector>

namespace orbis {
namespace fs = std::filesystem;

// Gestisce i riferimenti json-c con RAII; le copie condividono i nodi per evitare copie implicite
// costose.
class Json {
    std::shared_ptr<json_object> p;

  public:
    Json();
    explicit Json(json_object* value, bool retain = false);
    static Json parse(const std::string& text);
    static Json array();
    static Json string(const std::string& s);
    static Json number(double n);
    static Json boolean(bool b);
    Json get(const std::string& key) const;
    Json at(size_t index) const;
    void set(const std::string& key, const Json& value);
    void set(const std::string& key, const std::string& value);

    void set(const std::string& key, const char* value) {
        set(key, std::string(value));
    }

    void set(const std::string& key, double value);
    void set(const std::string& key, bool value);
    void push(const Json& value);
    bool has(const std::string& key) const;
    bool isArray() const;
    bool isObject() const;
    std::string str(const std::string& fallback = "") const;
    double num(double fallback = 0) const;
    bool flag(bool fallback = false) const;
    size_t size() const;
    std::string dump() const;

    Json clone() const {
        // Serializzazione e parsing producono una copia profonda per snapshot indipendenti tra
        // thread.
        return parse(dump());
    }
};

Json success(const std::string& message = "Operazione completata");
std::string readFile(const fs::path& path);
void writeFile(const fs::path& path, const std::string& data);
std::string now();
bool naturalLess(const std::string& a, const std::string& b);

class AsyncLogger;

class Store {
    sqlite3* db = nullptr;
    // Import/export richiamano metodi già protetti: il mutex ricorsivo mantiene un'unica sezione
    // critica.
    std::recursive_mutex mutex;

  public:
    fs::path root;
    ChangeNotifier changes;
    std::unique_ptr<AsyncLogger> logger;
    explicit Store(fs::path root);
    ~Store();
    Json list(const std::string& kind);
    Json get(const std::string& kind, long id);
    long save(const std::string& kind, Json value, long id = 0);
    void remove(const std::string& kind, long id);
    Json exportData();
    void importData(const Json& data);
    int cleanup(int months, long channel = 0);
    bool removeHistoryIfInactive(long id, const std::string& startedAt);
};

class Process {
    int pid = -1;
    int output = -1;
    int exitCode = 0;
    std::thread logReader;

  public:
    Process() = default;
    Process(const Process&) = delete;
    ~Process();
    void start(const std::vector<std::string>& args, int input = -1, const fs::path& log = {},
               bool capture = true, std::function<std::string(const std::string&)> redact = {});

    int fd() const {
        return output;
    }

    int id() const {
        return pid;
    }

    bool running();
    int wait();
    void stop();
};

std::string capture(const std::vector<std::string>& args, int timeoutSeconds = 15,
                    std::stop_token stop = {});
std::string ffmpeg();
std::string ffprobe();
Json probe(const std::string& file);
std::string redactStreamText(std::string message, const Json& request = Json{});
std::string readStreamLogs(const fs::path& root, long id);
std::vector<std::string> buildPublisher(const std::string& output);
Json sources();
Json metrics(const fs::path& root);
Json processMetrics(long pid);
std::vector<std::string> scanVideos(const fs::path& dir);
void validate(const std::string& kind, const Json& value);

struct Build {
    std::vector<std::string> args;
    double duration = 0;
    std::vector<std::shared_ptr<void>> resources;
};

Build buildProducer(const Json& scene, const Json& profile, double offset, const fs::path& preview);

struct FlvChunk {
    std::vector<unsigned char> bytes;
    uint64_t timestamp = 0;
    bool media = false;
    std::shared_ptr<const Json> context;
};

class FlvRelay {
    std::vector<unsigned char> pending;
    size_t cursor = 0;
    bool header = false, sentHeader = false, metadataSent = false;
    std::vector<unsigned char> audioConfiguration;
    uint64_t base = 0, last = 0;
    uint64_t first = 0, lastRaw = 0, wrap = 0, lastVideo = 0, frameStep = 33;
    bool firstTag = true;
    std::chrono::steady_clock::time_point epoch;

  public:
    void segment();
    std::vector<FlvChunk> packets(const unsigned char* bytes, size_t size);
    std::vector<unsigned char> feed(const unsigned char* bytes, size_t size, bool pace = true);

    uint64_t timestamp() const {
        return last;
    }

    uint64_t segmentBase() const {
        return base;
    }
};

class Session {
    Store& store;
    std::mutex mutex;
    std::mutex stopMutex;
    std::thread worker;
    std::atomic<bool> stopped{false}, finished{false}, changed{false};
    Json request, profile, original, outputProfile, videoResults, delivery;
    std::vector<Json> videoProfiles;
    std::vector<Json> playlist;
    int index = 0;
    double offset = 0;
    Json takeover;
    std::vector<Json> spots;
    std::string state = "STARTING", error, stopReason;
    int producerPid = -1, publisherPid = -1;
    int reconnectAttempts = 0;
    double outputKbps = 0;
    void run();

  public:
    long id, channel;
    fs::path frame;
    Session(Store& store, Json request, long id);
    ~Session();
    void start();
    void stop(const std::string& reason = "");

    bool done() const {
        return finished;
    }

    Json snapshot();
    void command(const std::string& name, const Json& data);
};

class LiveHistoryCleanupService;
class YouTubeAirState;

class Studio {
    std::mutex mutex;
    std::map<long, std::shared_ptr<Session>> sessions;
    std::jthread housekeeping, airWatcher;
    void checkYouTube(std::stop_token stop);

  public:
    Store store;
    std::unique_ptr<LiveHistoryCleanupService> historyCleanup;
    std::unique_ptr<YouTubeAirState> youtubeAir;
    explicit Studio(const fs::path& root);
    ~Studio();
    long start(Json request);
    std::shared_ptr<Session> session(long id);
    Json live();
    void dismissAir(long id);
    void stopAll();
};

void serve(Studio& studio, unsigned short port, const fs::path& web, const std::string& token);
} // namespace orbis
