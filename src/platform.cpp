#include <planet/platform.hpp>
#include <planet/serialise.hpp>

#include <felspar/exceptions/runtime_error.hpp>

#include <utility>


std::string_view planet::to_string(platform const p) noexcept {
    switch (p) {
    case platform::unknown: return "unknown";
    case platform::linux: return "linux";
    case platform::windows: return "windows";
    case platform::macos: return "macos";
    case platform::android: return "android";
    case platform::ios: return "ios";
    }
    std::unreachable();
}
planet::platform planet::platform_from_string(std::string_view const name) {
    if (name == "unknown") {
        return platform::unknown;
    } else if (name == "linux") {
        return platform::linux;
    } else if (name == "windows") {
        return platform::windows;
    } else if (name == "macos") {
        return platform::macos;
    } else if (name == "android") {
        return platform::android;
    } else if (name == "ios") {
        return platform::ios;
    } else {
        throw felspar::stdexcept::runtime_error{
                "Unknown platform name " + std::string{name}};
    }
}


void planet::save(serialise::save_buffer &sb, platform const p) {
    sb.save_box(platform_box, to_string(p));
}
void planet::load(serialise::box &b, platform &p) {
    std::string_view name;
    b.named(platform_box, name);
    p = platform_from_string(name);
}
