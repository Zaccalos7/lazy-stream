#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace orbis {
// Read configuration as data, never as shell commands. Existing process variables win.
inline void loadEnvironment(const std::filesystem::path& file, bool required = false) {
    std::ifstream input(file);
    if (!input) {
        if (required || std::filesystem::exists(file))
            throw std::runtime_error("Impossibile leggere env: " + file.string());
        return;
    }
    auto trim = [](std::string value) {
        auto first = value.find_first_not_of(" \t\r");
        return first == std::string::npos
                   ? std::string{}
                   : value.substr(first, value.find_last_not_of(" \t\r") - first + 1);
    };
    std::string line;
    size_t number = 0;
    while (std::getline(input, line)) {
        ++number;
        line = trim(line);
        if (line.empty() || line.front() == '#')
            continue;
        if (line.starts_with("export "))
            line = trim(line.substr(7));
        const auto equal = line.find('=');
        auto key = trim(line.substr(0, equal));
        if (equal == std::string::npos || key.empty() ||
            key.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != std::string::npos ||
            (key.front() >= '0' && key.front() <= '9'))
            throw std::runtime_error("Riga env non valida: " + file.string() + ":" +
                                     std::to_string(number));
        // Restrict file settings to the app; do not inject loader or shell variables.
        if (!key.starts_with("LAZY_STREAM_") && key != "FFMPEG_PATH" && key != "FFPROBE_PATH")
            throw std::runtime_error("Variabile env non supportata: " + key);
        auto value = trim(line.substr(equal + 1));
        if (!value.empty() && (value.front() == '\'' || value.front() == '"')) {
            if (value.size() < 2 || value.back() != value.front())
                throw std::runtime_error("Virgolette env non valide: " + key);
            value = value.substr(1, value.size() - 2);
        }
        if (!std::getenv(key.c_str()) && setenv(key.c_str(), value.c_str(), 0) != 0)
            throw std::runtime_error("Impossibile impostare env: " + key);
    }
}
} // namespace orbis
