#include "encoder_options.hpp"
#include <algorithm>
#include <sstream>
namespace orbis {
namespace {
std::string lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return std::tolower(c); });
    return text;
}
std::string name(const Json& option) {
    const auto key = option.get("key").str();
    return key.substr(0, key.find(':'));
}
void add(Json& options, const std::string& key, const std::string& value) {
    if (value.empty()) return;
    for (size_t i = 0; i < options.size(); ++i)
        if (name(options.at(i)) == key) return;
    Json option; option.set("key", key); option.set("value", value); options.push(option);
}
}
bool profileSwitch(const Json& profile, const std::string& field, const std::string& option) {
    if (profile.has(field)) return profile.get(field).flag(true);
    auto options = profile.get("videoSettingsOptions");
    std::string value;
    for (size_t i = 0; i < options.size(); ++i)
        if (options.at(i).get("key").str() == option) value = lower(options.at(i).get("value").str());
    return value != "0" && value != "false" && value != "no" && value != "off";
}
Json encoderOptions(const Json& profile, const std::string& encoder) {
    auto options = profile.get("videoSettingsOptions").clone();
    if (!options.isArray()) options = Json::array();
    if (!encoder.ends_with("_nvenc")) return options;
    if (profile.get("encoderQuality").str().empty() && profile.has("preset"))
        add(options, "preset", profile.get("preset").str());
    auto result = Json::array();
    std::string tune, multipass, rc;
    for (size_t i = 0; i < options.size(); ++i) {
        auto option = options.at(i).clone();
        const auto key = name(option), value = lower(option.get("value").str());
        if (key == "preset") {
            if (value == "default" || value == "ll" || value == "lossless") option.set("value", "p4");
            else if (value == "hp" || value == "llhp" || value == "losslesshp") option.set("value", "p1");
            else if (value == "hq" || value == "llhq") option.set("value", "p7");
            else if (value == "bd") option.set("value", "p5");
            if (value == "ll" || value == "llhp" || value == "llhq") tune = "ll";
            if (value == "lossless" || value == "losslesshp") tune = "lossless";
        } else if (key == "rc") {
            if (value == "vbr_minqp" || value == "vbr_2pass" || value == "vbr_hq") option.set("value", "vbr");
            else if (value == "ll_2pass_quality" || value == "ll_2pass_size" || value == "cbr_ld_hq" || value == "cbr_hq") option.set("value", "cbr");
            if (multipass.empty() && (value == "vbr_2pass" || value == "vbr_hq" || value == "ll_2pass_quality" || value == "cbr_ld_hq" || value == "cbr_hq")) multipass = "fullres";
            else if (multipass.empty() && value == "ll_2pass_size") multipass = "qres";
        } else if (key == "2pass" || key == "cbr") {
            if (key == "2pass") {
                if (value == "1" || value == "true") multipass = "qres";
                else if (value == "0" || value == "false") multipass = "disabled";
            } else if (value == "1" || value == "true") rc = "cbr";
            continue;
        }
        result.push(option);
    }
    add(result, "tune", tune); add(result, "multipass", multipass); add(result, "rc", rc);
    return result;
}
std::string x264Parameters(const Json& options, bool constantBitrate) {
    std::vector<std::pair<std::string, std::string>> parameters;
    auto set = [&](std::string key, std::string value) {
        for (auto& pair : parameters)
            if (pair.first == key) { pair.second = value; return; }
        parameters.emplace_back(std::move(key), std::move(value));
    };
    for (size_t i = 0; i < options.size(); ++i) {
        if (options.at(i).get("key").str() != "x264-params") continue;
        std::istringstream parts(options.at(i).get("value").str());
        std::string parameter;
        while (std::getline(parts, parameter, ':')) {
            const auto equal = parameter.find('=');
            if (equal == std::string::npos || equal == 0) throw std::runtime_error("Parametro x264 non valido");
            set(parameter.substr(0, equal), parameter.substr(equal + 1));
        }
    }
    if (constantBitrate) { set("nal-hrd", "cbr"); set("force-cfr", "1"); }
    std::string result;
    for (const auto& [key, value] : parameters)
        result += (result.empty() ? "" : ":") + key + "=" + value;
    return result;
}
} // namespace orbis

#ifdef ORBIS_FFMPEG9
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}
#endif
namespace orbis {
namespace {
std::string resolveEncoder(const Json& profile, bool video) {
    const char* field = video ? "videoCodecName" : "audioCodecName";
    const char* idField = video ? "videoCodec" : "audioCodec";
    auto selected = profile.get(field).str();
#ifdef ORBIS_FFMPEG9
    if (selected.empty() || (!video && profile.has(idField))) {
        const auto id = AVCodecID(int(profile.get(idField).num(video ? AV_CODEC_ID_H264 : AV_CODEC_ID_AAC)));
        if (const auto encoder = avcodec_find_encoder(id)) selected = encoder->name;
        else throw std::runtime_error("Encoder FFmpeg non disponibile per codec " + std::to_string(id));
    }
#endif
    if (selected.empty()) selected = video ? "libx264" : "aac";
    if (!std::all_of(selected.begin(), selected.end(), [](unsigned char c) { return std::isalnum(c) || c == '_'; }))
        throw std::runtime_error("Nome encoder non valido");
    return selected;
}
}
std::string videoEncoder(const Json& profile) { return resolveEncoder(profile, true); }
std::string audioEncoder(const Json& profile) { return resolveEncoder(profile, false); }
std::string videoPixelFormat(const Json& profile) {
    auto value = profile.get("pixelFormat").str("yuv420p");
#ifdef ORBIS_FFMPEG9
    if (!value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c); })) {
        const auto name = av_get_pix_fmt_name(AVPixelFormat(int(profile.get("pixelFormat").num())));
        if (!name) throw std::runtime_error("Formato pixel FFmpeg sconosciuto");
        value = name;
    }
#endif
    if (!std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isalnum(c) || c == '_'; }))
        throw std::runtime_error("Formato pixel non valido");
    return value;
}
bool encoderOptionSupported(const std::string& encoderName, const std::string& option) {
#ifdef ORBIS_FFMPEG9
    const auto base = option.substr(0, option.find(':'));
    if (const auto encoder = avcodec_find_encoder_by_name(encoderName.c_str())) {
        auto privateClass = encoder->priv_class;
        auto codecClass = avcodec_get_class();
        return (privateClass && av_opt_find(&privateClass, base.c_str(), nullptr, AV_OPT_FLAG_ENCODING_PARAM, AV_OPT_SEARCH_FAKE_OBJ)) ||
               av_opt_find(&codecClass, base.c_str(), nullptr, AV_OPT_FLAG_ENCODING_PARAM, AV_OPT_SEARCH_FAKE_OBJ);
    }
#else
    (void)encoderName; (void)option;
#endif
    return false;
}
} // namespace orbis
