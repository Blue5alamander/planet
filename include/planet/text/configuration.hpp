#pragma once


#include <planet/events/key_binding.hpp>
#include <planet/serialise/forward.hpp>


namespace planet::text {


    /// ## The keys an edit is driven by
    /**
     * One binding for each thing an edit can be told to do, which is what
     * `planet::text::editor` looks for in the key events it is handed. It is
     * a configuration rather than a table baked into the editor because which
     * key does what is the player's to choose, so it saves and loads
     * alongside the rest of a game's settings.
     *
     * The defaults are what a text field does everywhere: Return commits,
     * Escape abandons, Backspace and Delete take out the character on either
     * side of the caret, and Left, Right, Home and End move it. Nothing is
     * bound to Up or Down -- a single line has nowhere for them to go -- but
     * nothing here stops a game binding them either.
     *
     * **The modifiers held have to match exactly.** That is
     * `events::key_binding::matches`'s rule rather than this type's, and it
     * means a binding of Left alone is not also shift-Left: a combination a
     * game means something else by cannot fall into an edit that happens to
     * be running.
     *
     * A key bound to two things does the first of them. The editor tries the
     * bindings in the order they are declared here, so a configuration that
     * binds one key twice is answered rather than refused.
     */
    struct configuration final {
        static constexpr std::string_view box{"_p:text:configuration"};


        /// ### Ending an edit
        events::key_binding commit{events::scancode::return_key, {}};
        events::key_binding cancel{events::scancode::escape_key, {}};
        /**
         * The editor only reports these: what ending an edit does -- giving
         * up the focus, writing the value out -- needs more than the text, so
         * it belongs to whatever is driving the editor.
         */

        /// ### Removing text
        events::key_binding erase_backwards{
                events::scancode::backspace_key, {}};
        events::key_binding erase_forwards{events::scancode::delete_key, {}};
        /**
         * The character before the caret and the character after it, both
         * whole characters rather than single bytes.
         */

        /// ### Moving the caret
        events::key_binding caret_left{events::scancode::left_key, {}};
        events::key_binding caret_right{events::scancode::right_key, {}};
        events::key_binding caret_to_start{events::scancode::home_key, {}};
        events::key_binding caret_to_end{events::scancode::end_key, {}};


        constexpr bool
                operator==(configuration const &) const noexcept = default;
    };
    void save(serialise::save_buffer &, configuration const &);
    void load(serialise::box &, configuration &);


}
