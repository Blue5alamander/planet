#pragma once


#include <planet/events/keys.hpp>
#include <planet/events/text.hpp>

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
     * The editing is append-only: an edit starts with an empty buffer and
     * typing accumulates into it, with no caret and no correction. Events
     * arriving at rest are ignored, because a field stays subscribed whether
     * or not it is being edited -- it is the editor that discards them, and it
     * says so in what it returns.
     */
    class editor final {
        std::string current, before_edit;
        bool editing = false;


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
            default: return outcome::ignored;
            }
        }
    };


}
