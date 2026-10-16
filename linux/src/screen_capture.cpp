#include "screen_capture.hpp"
#include <cstdlib>
#include <cstring>
#include <regex>
#include <sstream>
#include <xcb/xcb.h>

namespace orbis {
namespace {
using Connection = std::unique_ptr<xcb_connection_t, decltype(&xcb_disconnect)>;

xcb_atom_t atom(xcb_connection_t* connection, const char* name) {
    auto reply = xcb_intern_atom_reply(
        connection, xcb_intern_atom(connection, 1, std::strlen(name), name), nullptr);
    const auto value = reply ? reply->atom : xcb_atom_t(XCB_ATOM_NONE);
    std::free(reply);
    return value;
}

std::string windowName(xcb_connection_t* connection, xcb_window_t window) {
    auto property = atom(connection, "_NET_WM_NAME");
    auto utf8 = atom(connection, "UTF8_STRING");
    auto reply = xcb_get_property_reply(
        connection, xcb_get_property(connection, 0, window, property, utf8, 0, 1024), nullptr);
    std::string text;
    if (reply && reply->format == 8)
        text.assign(static_cast<char*>(xcb_get_property_value(reply)),
                    xcb_get_property_value_length(reply));
    std::free(reply);
    if (text.empty()) {
        reply = xcb_get_property_reply(
            connection,
            xcb_get_property(connection, 0, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 1024),
            nullptr);
        if (reply && reply->format == 8)
            text.assign(static_cast<char*>(xcb_get_property_value(reply)),
                        xcb_get_property_value_length(reply));
        std::free(reply);
    }
    return text;
}
} // namespace

Json screenSources() {
    auto result = Json::array();
    const char* display = std::getenv("DISPLAY");
    if (!display || !*display)
        return result;
    Connection connection(xcb_connect(display, nullptr), xcb_disconnect);
    if (xcb_connection_has_error(connection.get()))
        return result;
    auto screens = xcb_setup_roots_iterator(xcb_get_setup(connection.get()));
    for (int index = 0; screens.rem; ++index, xcb_screen_next(&screens)) {
        auto screen = screens.data;
        auto sourceDisplay = std::string(display);
        const auto dot = sourceDisplay.find('.', sourceDisplay.find(':'));
        sourceDisplay = sourceDisplay.substr(0, dot) + "." + std::to_string(index);
        Json desktop;
        desktop.set("kind", "SCREEN");
        desktop.set("target", sourceDisplay);
        desktop.set("label", "Desktop " + std::to_string(index + 1));
        desktop.set("width", double(screen->width_in_pixels));
        desktop.set("height", double(screen->height_in_pixels));
        result.push(desktop);
        auto clients = xcb_get_property_reply(
            connection.get(),
            xcb_get_property(connection.get(), 0, screen->root,
                             atom(connection.get(), "_NET_CLIENT_LIST"), XCB_ATOM_WINDOW, 0, 4096),
            nullptr);
        if (!clients || clients->format != 32) {
            std::free(clients);
            continue;
        }
        auto windows = static_cast<xcb_window_t*>(xcb_get_property_value(clients));
        const int count = xcb_get_property_value_length(clients) / sizeof(xcb_window_t);
        for (int i = 0; i < count; ++i) {
            auto attributes = xcb_get_window_attributes_reply(
                connection.get(), xcb_get_window_attributes(connection.get(), windows[i]), nullptr);
            const bool visible = attributes && attributes->map_state == XCB_MAP_STATE_VIEWABLE;
            std::free(attributes);
            if (!visible)
                continue;
            const auto name = windowName(connection.get(), windows[i]);
            if (name.empty())
                continue;
            auto geometry = xcb_get_geometry_reply(
                connection.get(), xcb_get_geometry(connection.get(), windows[i]), nullptr);
            if (!geometry)
                continue;
            Json window;
            window.set("kind", "SCREEN");
            window.set("target", "x11-window:" + std::to_string(windows[i]) + "@" + sourceDisplay);
            window.set("label", name);
            window.set("width", double(geometry->width));
            window.set("height", double(geometry->height));
            result.push(window);
            std::free(geometry);
        }
        std::free(clients);
    }
    try {
        std::istringstream lines(capture({"xrandr", "--listmonitors"}, 3));
        const std::regex monitor(R"(\s*\d+:\s+\S+\s+(\d+)/\d+x(\d+)/\d+\+(\d+)\+(\d+)\s+(.+))");
        std::string line;
        while (std::getline(lines, line)) {
            std::smatch match;
            if (!std::regex_match(line, match, monitor))
                continue;
            Json source;
            source.set("kind", "SCREEN");
            source.set("target", "x11-monitor:" + match[3].str() + "," + match[4].str() + "," +
                                     match[1].str() + "," + match[2].str() + "@" + display);
            source.set("label", match[5].str());
            source.set("width", std::stod(match[1]));
            source.set("height", std::stod(match[2]));
            result.push(source);
        }
    } catch (...) {
    }
    return result;
}

std::vector<std::string> screenInput(const std::string& target, double fps) {
    std::vector<std::string> input{"-thread_queue_size", "512", "-f", "x11grab", "-framerate",
                                   std::to_string(fps)};
    auto display = target;
    const auto separator = target.rfind('@');
    if (target.starts_with("x11-window:") && separator != std::string::npos) {
        const auto id = target.substr(11, separator - 11);
        if (!std::regex_match(id, std::regex("[0-9]{1,10}")) || std::stoull(id) > UINT32_MAX ||
            std::stoull(id) == 0)
            throw std::runtime_error("Finestra X11 non valida");
        input.insert(input.end(), {"-window_id", id});
        display = target.substr(separator + 1);
    } else if (target.starts_with("x11-monitor:") && separator != std::string::npos) {
        std::smatch match;
        const auto bounds = target.substr(12, separator - 12);
        if (!std::regex_match(bounds, match,
                              std::regex("([0-9]{1,5}),([0-9]{1,5}),([0-9]{1,5}),([0-9]{1,5})")) ||
            std::stoi(match[3]) < 1 || std::stoi(match[4]) < 1)
            throw std::runtime_error("Schermo X11 non valido");
        input.insert(input.end(), {"-grab_x", match[1], "-grab_y", match[2], "-video_size",
                                   match[3].str() + "x" + match[4].str()});
        display = target.substr(separator + 1);
    }
    if (!std::regex_match(display, std::regex(":[0-9]+(?:\\.[0-9]+)?")))
        throw std::runtime_error("DISPLAY X11 non valido");
    input.insert(input.end(), {"-i", display});
    return input;
}
} // namespace orbis
