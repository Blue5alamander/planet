#pragma once


#include <planet/serialise/base_types.hpp>
#include <planet/serialise/load_buffer.hpp>
#include <planet/serialise/save_buffer.hpp>
#include <planet/events.hpp>

#include <utility>


namespace planet::events {


    /// ## `planet::events::back`
    inline void save(serialise::save_buffer &ab, back) {
        ab.save_box(back::box);
    }
    inline void load(serialise::load_buffer &lb, back &) {
        lb.load_box(back::box);
    }


    /// ## `planet::events::key_binding`
    inline void save(serialise::save_buffer &ab, key_binding const &kb) {
        ab.save_box(key_binding::box, kb.scancode, kb.modifiers);
    }
    inline void load(serialise::box &b, key_binding &kb) {
        b.named(key_binding::box, kb.scancode, kb.modifiers);
    }


    /// ## `planet::events::modifiers`
    inline void save(serialise::save_buffer &ab, modifiers const &m) {
        ab.save_box(modifiers::box, m.shift, m.ctrl, m.alt, m.gui);
    }
    inline void load(serialise::box &b, modifiers &m) {
        b.named(modifiers::box, m.shift, m.ctrl, m.alt, m.gui);
    }


    /// ## `planet::events::scancode`
    inline void save(serialise::save_buffer &ab, scancode const s) {
        serialise::save(ab, std::to_underlying(s));
    }
    inline void load(serialise::load_buffer &lb, scancode &s) {
        s = static_cast<scancode>(
                serialise::load_type<std::underlying_type_t<scancode>>(lb));
    }
    /**
     * Saved as its number rather than a name because the numbers are the USB
     * HID ones, which the platform layer passes straight through: every key
     * on the keyboard has one whether or not the enumeration has got around
     * to naming it, and a binding must survive being saved either way.
     */


    /// ## `planet::events::quit`
    inline void save(serialise::save_buffer &ab, quit) {
        ab.save_box(quit::box);
    }
    inline void load(serialise::load_buffer &lb, quit &) {
        lb.load_box(quit::box);
    }


}
