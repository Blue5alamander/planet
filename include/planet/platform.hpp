#pragma once


#include <planet/serialise/forward.hpp>

#include <string_view>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif


/**
 * The GNU language dialects (e.g. `-std=gnu++23`) pre-define `linux` as a
 * macro for historical reasons. Portable code must use `__linux__` instead, so
 * the macro is safe to remove, which allows its use as an enumerator name.
 */
#undef linux


namespace planet {


    /// ## The platforms the game can be built for

    enum class platform { unknown, linux, windows, macos, android, ios };

    inline std::string_view constexpr platform_box{"_p:platform"};


    /// ### The platform this code was built for
    inline platform constexpr current_platform =
#if defined(__ANDROID__)
            platform::android;
#elif defined(__linux__)
            platform::linux;
#elif defined(_WIN32)
            platform::windows;
#elif defined(__APPLE__) and TARGET_OS_IPHONE
            platform::ios;
#elif defined(__APPLE__)
            platform::macos;
#else
            platform::unknown;
#endif


    /// ### Platform categories
    /**
     * `unknown` is deliberately neither desktop nor mobile, so both queries
     * return `false` for it.
     */
    [[nodiscard]] inline bool constexpr is_mobile(
            platform const p = current_platform) noexcept {
        return p == platform::android or p == platform::ios;
    }
    [[nodiscard]] inline bool constexpr is_desktop(
            platform const p = current_platform) noexcept {
        return p == platform::linux or p == platform::windows
                or p == platform::macos;
    }


    /// ### Convert a `platform` to and from its name
    std::string_view to_string(platform) noexcept;
    platform platform_from_string(std::string_view);
    /**
     * `platform_from_string` throws if the name is not recognised.
     */


    /// ### Save and load a `platform`
    void save(serialise::save_buffer &, platform);
    void load(serialise::box &, platform &);
    /**
     * The platform is boxed under `platform_box` carrying its name.
     */


}
