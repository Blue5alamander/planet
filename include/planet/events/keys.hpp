#pragma once


#include <planet/events/action.hpp>

#include <chrono>


namespace planet::events {


    /// ## Enumeration for keyboard scan codes
    /**
     * Scan codes describe the keys on the keyboard in a locale independent
     * manner. This means that no matter the keyboard layout the scan codes will
     * always be the same.
     *
     * We copy SDL in that we'll set the scan codes to be the ones defined in
     * the USB HID specification's _"Keyboard/Keypad Page (0x07)"_
     * <https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf>
     */
    enum class scancode : std::uint16_t {
        none = 0,

        letter_a = 4,
        letter_b = 5,
        letter_c = 6,
        letter_d = 7,
        letter_e = 8,
        letter_f = 9,
        letter_g = 10,
        letter_h = 11,
        letter_i = 12,
        letter_j = 13,
        letter_k = 14,
        letter_l = 15,
        letter_m = 16,
        letter_n = 17,
        letter_o = 18,
        letter_p = 19,
        letter_q = 20,
        letter_r = 21,
        letter_s = 22,
        letter_t = 23,
        letter_u = 24,
        letter_v = 25,
        letter_w = 26,
        letter_x = 27,
        letter_y = 28,
        letter_z = 29,

        number_1 = 30,
        number_2 = 31,
        number_3 = 32,
        number_4 = 33,
        number_5 = 34,
        number_6 = 35,
        number_7 = 36,
        number_8 = 37,
        number_9 = 38,
        number_0 = 39,

        return_key = 40,
        escape_key = 41,
        backspace_key = 42,
        tab_key = 43,
        space_key = 44,

        insert_key = 73,
        home_key = 74,
        page_up_key = 75,
        delete_key = 76,
        end_key = 77,
        page_down_key = 78,
    };


    /// ## Modifier keys held during a key event
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


    /// ## A key/button press/release event
    struct key final {
        events::scancode scancode = events::scancode::none;
        events::action action = events::action::released;
        events::modifiers modifiers = {};
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::now();
    };


}
