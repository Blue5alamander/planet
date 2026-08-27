#pragma once


#include <planet/events/modifiers.hpp>
#include <planet/platform.hpp>


namespace planet::events {


    /// ## The modifier that marks a keystroke as an application command
    [[nodiscard]] constexpr modifiers control(modifiers extra = {}) noexcept
    /**
     * The key a platform reserves for driving the program -- copy, undo, and
     * the rest -- rather than for typing into it. That key is Control on
     * Windows and Linux, and Command (the `gui` key) on macOS and iOS.
     *
     * `extra` names any other modifiers to hold alongside it -- Shift for a
     * redo (Ctrl-Shift-Z), say -- and comes back with the correct command
     * modifier:
     *
     * ```cpp
     * key_binding copy{scancode::letter_c, control()};
     * key_binding redo{scancode::letter_z, control({.shift = true})};
     * ```
     */
    {
        bool constexpr apple =
                (planet::current_platform == planet::platform::macos
                 or planet::current_platform == planet::platform::ios);
        if constexpr (apple) {
            extra.ctrl = false;
            extra.gui = true;
        } else {
            extra.ctrl = true;
            extra.gui = false;
        }
        return extra;
    }


}
