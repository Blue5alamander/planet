#pragma once


namespace planet::events {


    /// ## Modifier keys held during an input event
    /**
     * Each flag is true when either the left or right modifier of that kind
     * was held when the event happened. The state travels with the event
     * rather than being tracked from the modifiers' own key events, so a
     * consumer never sees a stale reading when a press or release was routed
     * elsewhere.
     *
     * `gui` is the USB HID name for the operating system's own key: the
     * Windows key, the Command key on macOS, or Super/Meta on Linux.
     *
     * To match a keyboard shortcut, construct the combination you want and
     * compare the whole value, e.g. `key.modifiers == modifiers{.ctrl =
     * true}`. Reading the flags individually silently accepts any modifier
     * you forgot to check, so a shortcut reserved for another meaning (say
     * Meta-Ctrl where Ctrl was intended) triggers too.
     */
    struct modifiers final {
        bool shift = false;
        bool ctrl = false;
        bool alt = false;
        bool gui = false;

        constexpr bool operator==(modifiers const &) const noexcept = default;
    };


}
