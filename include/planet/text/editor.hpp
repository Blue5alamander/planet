#pragma once


#include <planet/events/keys.hpp>
#include <planet/events/text.hpp>
#include <planet/text/boundary.hpp>

#include <string>
#include <utility>


namespace planet::text {


    /// ## What handing an event to an editor did
    /**
     * The editor reports what an event meant and leaves acting on it to
     * whatever is driving it. The two outcomes that end an edit --
     * `commit` and `cancel` -- say only that the key is bound to that: the side
     * effects of ending one (giving up the focus, writing the value out,
     * telling the platform to put its keyboard away) need a baseplate, so they
     * belong to `planet::widget::text_input` rather than here. The editor is
     * still editing until `commit()` or `cancel()` is called on it.
     */
    enum class outcome { ignored, changed, commit, cancel };


    /// ## Single line text editing
    /**
     * The text, and nothing else: no graphic, no size, no position, no z layer
     * and no baseplate. An edit is begun, driven by the events handed to it,
     * and then either committed or cancelled.
     *
     * ```mermaid
     * stateDiagram-v2
     *     [*] --> resting
     *     resting --> editing: begin()
     *     editing --> resting: commit() -- keep what was typed
     *     editing --> resting: cancel() -- restore the previous value
     * ```
     *
     * The editing happens at the end of the buffer: an edit starts empty,
     * typing accumulates onto the end of it and backspace takes the last
     * character back off, with no caret and so no way to correct anywhere
     * else. Events arriving at rest are ignored, because a field stays
     * subscribed whether
     * or not it is being edited -- it is the editor that discards them, and it
     * says so in what it returns.
     */
    class editor final {
        std::string current, before_edit;
        bool editing = false;


        /// Take the character off the end of the buffer
        /**
         * The whole character, never the last byte of one: what is left has to
         * still be text. A buffer with nothing in it reports that nothing
         * changed, which is what tells whatever is driving the editor that no
         * redraw is due.
         */
        outcome erase_backwards() {
            if (current.empty()) { return outcome::ignored; }
            current.erase(previous_boundary(current, current.size()));
            return outcome::changed;
        }


      public:
        using value_type = std::string;


        /// ### Construction
        explicit editor(value_type v = {}) : current{std::move(v)} {}


        /// ### Observable state

        /// #### Whether an edit is running
        bool is_editing() const noexcept { return editing; }

        /// #### The text to show
        /**
         * The value at rest, and the live buffer while an edit is running.
         */
        value_type const &value() const noexcept { return current; }


        /// ### The edit

        /// #### Begin an edit
        void begin() {
            before_edit = std::exchange(current, {});
            editing = true;
        }

        /// #### Keep what was typed
        void commit() {
            before_edit.clear();
            editing = false;
        }

        /// #### Abandon the edit and restore what was there before it
        void cancel() {
            current = std::move(before_edit);
            before_edit.clear();
            editing = false;
        }


        /// ### Events

        /// #### Characters that have been typed
        outcome handle(events::text const &t) {
            if (not editing) { return outcome::ignored; }
            current += t.utf8;
            return outcome::changed;
        }

        /// #### The keys an edit is controlled with
        /**
         * Only a key going down counts. A key held from before the edit began
         * releases into it, and that release must not be what ends it.
         */
        outcome handle(events::key const &k) {
            if (not editing or k.action != events::action::down) {
                return outcome::ignored;
            }
            switch (k.scancode) {
            case events::scancode::return_key: return outcome::commit;
            case events::scancode::escape_key: return outcome::cancel;
            case events::scancode::backspace_key: return erase_backwards();
            default: return outcome::ignored;
            }
        }
    };


}
