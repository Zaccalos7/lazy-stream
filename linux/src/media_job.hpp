#pragma once
#include "core.hpp"

namespace orbis {
class MediaJob {
    std::mutex mutex;
    std::thread worker;
    std::atomic<bool> cancelled{false};
    std::string status = "RUNNING", error;
    double seconds = 0;

  public:
    fs::path output;
    MediaJob(std::vector<std::string> args, fs::path output);
    ~MediaJob();
    Json snapshot();
};

class ProxyLibrary {
    std::mutex mutex;
    std::map<std::string, std::shared_ptr<MediaJob>> jobs;
    fs::path root;

  public:
    explicit ProxyLibrary(fs::path root) : root(std::move(root)) {
        fs::create_directories(this->root);
    }

    Json prepare(const std::string& target);
    Json list();
};

Json platformPlayer(const Json& channel);
Json tuneEncoder(const Json& profile);
} // namespace orbis
