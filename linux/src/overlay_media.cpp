#include "overlay_media.hpp"
#include <fstream>
#include <array>
#include <cstring>
#include <unistd.h>

namespace orbis {
OverlayMedia OverlayMedia::of(const Json& probe) {
    auto streams = probe.get("streams");
    Json video;
    for (size_t i = 0; i < streams.size(); ++i)
        if (streams.at(i).get("codec_type").str() == "video") {
            video = streams.at(i);
            break;
        }
    const auto codec = video.get("codec_name").str();
    const auto format = probe.get("format").get("format_name").str();
    if (codec == "webp_anim" || format == "webp_anim")
        return {true, true, {}};
    if (format == "image2" || format.ends_with("_pipe") ||
        (format == "gif" && video.get("nb_frames").num() <= 1 &&
         probe.get("format").get("duration").num() <= .1))
        return {};
    const bool alpha = video.get("tags").get("alpha_mode").str() == "1" ||
                       video.get("tags").get("ALPHA_MODE").str() == "1";
    return {true, false, alpha && codec == "vp9" ? "libvpx-vp9" :
                        alpha && codec == "vp8" ? "libvpx" : ""};
}

WebpOverlayInput::WebpOverlayInput(const fs::path& source) : path(source) {
    std::ifstream file(source, std::ios::binary);
    auto read = [&](char* bytes, size_t count) {
        if (!file.read(bytes, count))
            throw std::runtime_error("WebP overlay troncato");
    };
    std::array<char, 12> header;
    read(header.data(), header.size());
    if (std::memcmp(header.data(), "RIFF", 4) || std::memcmp(header.data() + 8, "WEBP", 4))
        throw std::runtime_error("Header RIFF/WEBP non valido");
    const auto length = fs::file_size(source);
    auto little = [](const char* p) {
        const auto* b = reinterpret_cast<const unsigned char*>(p);
        return uint32_t(b[0]) | uint32_t(b[1]) << 8 | uint32_t(b[2]) << 16 | uint32_t(b[3]) << 24;
    };
    while (uint64_t(file.tellg()) + 8 <= length) {
        std::array<char, 8> chunk;
        read(chunk.data(), chunk.size());
        const uint64_t size = little(chunk.data() + 4), start = file.tellg();
        if (size > length - start)
            break;
        if (!std::memcmp(chunk.data(), "ANIM", 4) && size == 6) {
            file.seekg(start + 4);
            char count[2];
            read(count, 2);
            if (!count[0] && !count[1])
                return;
            auto pattern = (fs::temp_directory_path() / "orbis-webp-XXXXXX").string();
            if (!mkdtemp(pattern.data()))
                throw std::runtime_error("Impossibile preparare l'overlay WebP");
            directory = pattern;
            try {
                path = directory / "loop.webp";
                fs::copy_file(source, path);
                std::fstream output(path, std::ios::in | std::ios::out | std::ios::binary);
                output.seekp(start + 4);
                const char zero[2] = {};
                output.write(zero, 2);
                if (!output)
                    throw std::runtime_error("Impossibile scrivere l'overlay WebP");
                return;
            } catch (...) {
                std::error_code ignored;
                fs::remove_all(directory, ignored);
                throw;
            }
        }
        file.seekg(start + size + (size & 1));
    }
    throw std::runtime_error("Chunk ANIM WebP non valido");
}
WebpOverlayInput::~WebpOverlayInput() {
    if (!directory.empty()) {
        std::error_code ignored;
        fs::remove_all(directory, ignored);
    }
}
} // namespace orbis
