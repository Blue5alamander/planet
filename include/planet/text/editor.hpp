#pragma once


#include <planet/events/keys.hpp>
#include <planet/events/text.hpp>
#include <planet/text/boundary.hpp>
#include <planet/text/configuration.hpp>

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
     * already there with the caret at the end of it. Typing inserts at the
     * caret; everything else an edit can be told to do arrives as a key, and
     * which key does which is `keys` rather than anything fixed here.
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
     *
     * The two things that decide what an edit may do -- which key does what,
     * and what the value is allowed to be -- both come from outside, and are
     * given when the editor is made rather than set on it afterwards, so there
     * is no moment in which one can be typed into while it still answers to
     * the wrong keys or lets through a value the call site would have refused.
     */
    class editor final {
        configuration const &keys;
        std::function<bool(std::string_view)> acceptable;
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
            /// Consulted on the change; a removal is never blocked
            if (acceptable) { acceptable(current); }
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
            /// Consulted on the change; a removal is never blocked
            if (acceptable) { acceptable(current); }
            return outcome::changed;
        }


      public:
        using value_type = std::string;


        /// ### Construction

        /// #### Which key does what, and what is being edited
        /**
         * `k` says which key does what. It is held **by reference**: a game
         * keeps one configuration alongside the rest of its settings, and
         * every field built from it answers to that one, so a key the player
         * rebinds takes effect in each of them without any of them being
         * reached back into. It has to outlive the editor --
         * `planet::text::default_bindings` is there for a call site with
         * nothing of its own to say.
         *
         * Anything may be typed into an editor made this way. Most values
         * being edited have some rule about what they may be, and one that
         * does is made with the constructor below rather than by this one and
         * a rule handed over afterwards.
         */
        explicit editor(configuration const &k, value_type v = {})
        : keys{k}, current{std::move(v)} {}

        /// #### With a rule about what the value is allowed to be
        /**
         * `a` is consulted with the value the buffer *would* become before
         * anything is inserted into it, and again with what an edit is about
         * to be committed as. A `false` means the insertion does not happen,
         * or that the commit becomes a cancel: what an edit leaves behind is
         * always something this said yes to.
         *
         * The editor has no opinion of its own about what text is allowed. How
         * long a name may be, what characters it may contain, whether it may be
         * blank at all -- those are rules about the thing being edited rather
         * than about editing, so they come from the call site.
         *
         * A removal is consulted the same way, with the value the buffer has
         * become, but its answer is not obeyed there: a deletion is never
         * blocked, so a cap on how long the value grows cannot also stop it
         * being cut back down, and a rule that it may not be blank still leaves
         * a way to clear it and start again -- refusing the commit holds that
         * line instead. The consultation is for the reaction, not the veto: a
         * call site keeping something on the value as it is edited -- a slider
         * on the number it spells -- hears a character come out as much as go
         * in.
         */
        explicit editor(
                configuration const &k,
                std::function<bool(std::string_view)> a,
                value_type v = {})
        : keys{k}, acceptable{std::move(a)}, current{std::move(v)} {}


        /// ### Observable state

        /// #### Whether an edit is running
        bool is_editing() const noexcept { return editing; }

        /// #### The text to show
        /**
         * The value at rest, and the live buffer while an edit is running.
         */
        value_type const &value() const noexcept { return current; }

        /// #### Replace the value shown at rest
        /**
         * A resting value handed in from outside, so a field mirroring a
         * number some other control is changing keeps its text current.
         * Ignored while an edit is running: the live buffer being typed is
         * never overwritten by what the field is told it now reads.
         */
        void value(value_type v) noexcept {
            if (not editing) { current = std::move(v); }
        }

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
         * The bindings are tried in the order `text::configuration` declares
         * them, so a key bound to two things does the first of them. A key
         * bound to nothing reports as ignored, which is how the arrow keys
         * stay available to whatever else is listening for them when no edit
         * is running.
         */
        outcome handle(events::key const &k) {
            if (not editing or k.action != events::action::down) {
                return outcome::ignored;
            }
            if (keys.commit.matches(k)) {
                return outcome::commit;
            } else if (keys.cancel.matches(k)) {
                return outcome::cancel;
            } else if (keys.erase_backwards.matches(k)) {
                return erase_backwards();
            } else if (keys.erase_forwards.matches(k)) {
                return erase_forwards();
            } else if (keys.caret_left.matches(k)) {
                return move_caret_to(previous_boundary(current, caret));
            } else if (keys.caret_right.matches(k)) {
                return move_caret_to(next_boundary(current, caret));
            } else if (keys.caret_to_start.matches(k)) {
                return move_caret_to(0);
            } else if (keys.caret_to_end.matches(k)) {
                return move_caret_to(current.size());
            } else {
                return outcome::ignored;
            }
        }
    };


}
