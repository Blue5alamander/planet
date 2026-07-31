#pragma once


#include <planet/events/keys.hpp>
#include <planet/events/text.hpp>
#include <planet/text/boundary.hpp>

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
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
     *     editing --> resting: commit() refused -- restore the previous value
     *     editing --> resting: cancel() -- restore the previous value
     * ```
     *
     * An edit corrects rather than retypes: it begins on the value that is
     * already there with the caret at the end of it. Left, Right, Home and End
     * move the caret, typing inserts at it, backspace takes out the character
     * before it and Delete the one after.
     *
     * **The caret is a byte offset that always sits on a character boundary.**
     * It only ever moves to somewhere `previous_boundary` or `next_boundary`
     * handed back, or along by the bytes of whole characters inserted, so no
     * operation can leave it inside a character -- which is what lets a
     * presentation take the text before it as a view straight into the buffer.
     * `set_cursor` is where an offset arriving from outside is made to obey
     * that.
     *
     * Events arriving at rest are ignored, because a field stays subscribed
     * whether or not it is being edited -- it is the editor that discards them,
     * and it says so in what it returns.
     */
    class editor final {
        std::string current, before_edit;
        std::size_t caret = {};
        bool editing = false;


        /// Move the caret, saying whether it went anywhere
        /**
         * A caret already where it is being sent has not changed, which is what
         * tells whatever is driving the editor that no redraw is due.
         */
        outcome move_caret_to(std::size_t const to) noexcept {
            if (to == caret) { return outcome::ignored; }
            caret = to;
            return outcome::changed;
        }


        /// Take the character before the caret out of the buffer
        /**
         * The whole character, never the last byte of one: what is left has to
         * still be text. The caret follows the text it removed. Nothing sits
         * before the start of the buffer, so a caret there reports that nothing
         * changed.
         *
         * `std::string::erase` takes a count rather than an end offset, so the
         * difference between the two boundaries is spelled out.
         */
        outcome erase_backwards() {
            auto const from = previous_boundary(current, caret);
            if (from == caret) { return outcome::ignored; }
            current.erase(from, caret - from);
            caret = from;
            return outcome::changed;
        }

        /// Take the character after the caret out of the buffer
        /**
         * The text moves back to the caret, so the caret itself stays put.
         * Nothing sits after the end of the buffer.
         */
        outcome erase_forwards() {
            auto const to = next_boundary(current, caret);
            if (to == caret) { return outcome::ignored; }
            current.erase(caret, to - caret);
            return outcome::changed;
        }


      public:
        using value_type = std::string;


        /// ### Construction
        explicit editor(value_type v = {}) : current{std::move(v)} {}


        /// ### What the value is allowed to be
        std::function<bool(std::string_view)> acceptable;
        /**
         * Consulted with the value the buffer *would* become before anything
         * is inserted into it, and again with what an edit is about to be
         * committed as. A `false` means the insertion does not happen, or that
         * the commit becomes a cancel: what an edit leaves behind is always
         * something this said yes to.
         *
         * The editor has no opinion of its own about what text is allowed. How
         * long a name may be, what characters it may contain, whether it may be
         * blank at all -- those are rules about the thing being edited rather
         * than about editing, so they come from the call site. Left unbound,
         * anything goes.
         *
         * Deletion is deliberately not filtered. A cap that stopped the value
         * growing must not also stop it being cut back down, and a rule that it
         * may not be blank has to leave a way to clear it and start again --
         * refusing the commit is what holds that line instead.
         */


        /// ### Observable state

        /// #### Whether an edit is running
        bool is_editing() const noexcept { return editing; }

        /// #### The text to show
        /**
         * The value at rest, and the live buffer while an edit is running.
         */
        value_type const &value() const noexcept { return current; }

        /// #### Where the next character typed will go
        /**
         * A byte offset into `value()`, always on a character boundary.
         */
        std::size_t cursor() const noexcept { return caret; }


        /// ### The edit

        /// #### Begin an edit
        /**
         * The buffer starts as the value with the caret at the end of it, so an
         * edit begins by correcting what is there rather than by replacing it.
         */
        void begin() {
            before_edit = current;
            caret = current.size();
            editing = true;
        }

        /// #### Keep what was typed, if it is allowed to be kept
        /**
         * Reports whether it was. A value `acceptable` refuses is not something
         * an edit may leave behind, and the only other text there is to leave
         * is what was there before the edit, so a refused commit cancels
         * instead -- and whatever is driving the editor must not then write the
         * value out anywhere.
         */
        bool commit() {
            if (acceptable and not acceptable(current)) {
                cancel();
                return false;
            }
            before_edit.clear();
            editing = false;
            return true;
        }

        /// #### Abandon the edit and restore what was there before it
        void cancel() {
            current = std::move(before_edit);
            before_edit.clear();
            /**
             * The caret was an offset into the buffer that has just been thrown
             * away, and could be past the end of the one that replaced it.
             */
            caret = current.size();
            editing = false;
        }

        /// #### Put the caret at an offset from outside
        /**
         * What a presentation that has measured its way to an offset calls --
         * clicking into the middle of the text. It clamps to the buffer and
         * snaps onto a boundary, so the worst a mis-measurement can do is put
         * the caret a character out; it can never leave it inside one.
         *
         * The snap is a round trip through both boundary functions, because
         * each is exclusive of the offset it is handed and so neither on its
         * own can answer *at* one: the largest boundary before the smallest
         * boundary after `o` is the largest boundary at or before `o`.
         */
        void set_cursor(std::size_t const o) noexcept {
            caret = o >= current.size()
                    ? current.size()
                    : previous_boundary(current, next_boundary(current, o));
        }


        /// ### Events

        /// #### Characters that have been typed
        /**
         * They go in at the caret, which then follows them along by however
         * many bytes they took.
         *
         * The filter sees the whole value as it would be, so it is written as a
         * rule about the value rather than about the keystroke. What it refuses
         * is taken straight back out again -- all of it, however many bytes it
         * was -- leaving the buffer and the caret exactly where the keystroke
         * found them.
         */
        outcome handle(events::text const &t) {
            if (not editing) { return outcome::ignored; }
            current.insert(caret, t.utf8);
            if (acceptable and not acceptable(current)) {
                current.erase(caret, t.utf8.size());
                return outcome::ignored;
            }
            caret += t.utf8.size();
            return outcome::changed;
        }

        /// #### The keys an edit is controlled and navigated with
        /**
         * Only a key going down counts. A key held from before the edit began
         * releases into it, and that release must not be what ends it.
         *
         * Up and down are deliberately not bound: a single line has nowhere for
         * them to go, so they report as ignored like any other key the editor
         * has no use for.
         */
        outcome handle(events::key const &k) {
            if (not editing or k.action != events::action::down) {
                return outcome::ignored;
            }
            switch (k.scancode) {
            case events::scancode::return_key: return outcome::commit;
            case events::scancode::escape_key: return outcome::cancel;
            case events::scancode::backspace_key: return erase_backwards();
            case events::scancode::delete_key: return erase_forwards();
            case events::scancode::left_key:
                return move_caret_to(previous_boundary(current, caret));
            case events::scancode::right_key:
                return move_caret_to(next_boundary(current, caret));
            case events::scancode::home_key: return move_caret_to(0);
            case events::scancode::end_key:
                return move_caret_to(current.size());
            default: return outcome::ignored;
            }
        }
    };


}
