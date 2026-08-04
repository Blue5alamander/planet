#pragma once


#include <planet/events/keys.hpp>

#include <string_view>


namespace planet::events {


    /// ## A key combination an action can be bound to
    /**
     * The part of a `key` event that says *which* combination the user
     * pressed, separated from everything belonging to one particular press:
     * the `action` telling a press from a release, and the `timestamp` of
     * when it happened. A `key` always carries both -- its timestamp is
     * filled in with `steady_clock::now()` as the event is constructed -- so
     * an event can never stand in for a binding, which is also why `key` has
     * no equality operator to compare one with.
     *
     * There are deliberately no defaults. A binding is always a choice of
     * both a key and the modifiers that must be held with it, so a field
     * left out is a mistake rather than a sensible "no key" or "no
     * modifiers", and the compiler is left free to say so.
     */
    struct key_binding final {
        static constexpr std::string_view box{"_p:e:key_binding"};


        events::scancode scancode;
        events::modifiers modifiers;


        /// ### Is this key event the bound combination?
        constexpr bool matches(events::key const &k) const noexcept
        /**
         * The event's `action` is not considered -- a binding says which keys
         * are involved, not what happened to them -- so a caller that wants
         * only key downs checks `action` itself.
         *
         * The modifiers must match exactly, in line with how `modifiers`
         * itself is meant to be compared, so a binding of ctrl alone does not
         * fire when the player presses meta-ctrl and means something else by
         * it.
         */
        {
            return k.scancode == scancode and k.modifiers == modifiers;
        }

        constexpr bool operator==(key_binding const &) const noexcept = default;
    };


    /// ## The binding a key event matches
    constexpr key_binding binding_for(key const &k) noexcept
    /**
     * Capturing what the player just pressed, which is what a screen for
     * re-binding a key needs. To test an event against a binding that is
     * already known go the other way, with `key_binding::matches`, which
     * doesn't have to build a value to compare against.
     */
    {
        return {k.scancode, k.modifiers};
    }


}
