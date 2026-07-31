#pragma once


#include <planet/queue/psc.hpp>
#include <planet/text/editor.hpp>
#include <planet/ui/screen.hpp>
#include <planet/ui/widget.hpp>

#include <felspar/coro/future.hpp>


namespace planet::widget {


    /// ## Single line text input
    /**
     * The event shell around a `planet::text::editor`. It **draws nothing**:
     * everything it does is routing and focus, and every question about what
     * the text is it asks the editor.
     *
     * ```mermaid
     * stateDiagram-v2
     *     [*] --> resting
     *     resting --> editing: left click
     *     editing --> resting: return_key -- write the buffer to the output
     *     editing --> resting: escape_key -- restore the previous value
     *     editing --> resting: a click the dismissal screen takes --
     *         write the buffer
     *     editing --> resting: the editor refuses the value --
     *         restore it and write nothing
     * ```
     *
     * An edit holds the hard focus for as long as it runs, which puts the
     * field at the top of the baseplate's delivery stack so typing cannot leak
     * into whatever key controls sit beneath it.
     *
     * It captures the keyboard kinds only. A captured click would arrive here
     * whatever it was aimed at, leaving nothing to say whether it landed on the
     * field or somewhere else, and that is the difference between putting the
     * caret somewhere and ending the edit. Left uncaptured the mouse routes by
     * position: over the field it reaches the field, anywhere else it reaches
     * the dismissal screen.
     *
     * Ending an edit with the mouse is that screen's job, following the pattern
     * `b5::ui::singleselect` uses for its open drop down. While an edit runs
     * the field draws a `planet::ui::screen` beneath itself and lifts itself
     * above it, so the screen is the topmost widget everywhere the field is
     * not. The screen is a hover boundary, so the click stops there: a click
     * that finishes an edit dismisses, it does not also press whatever was
     * underneath it.
     *
     * Having no graphic, the shell has nothing to size itself from either: it
     * reflows to the constraint it is handed and covers the rectangle it is
     * handed. The presentation lays its own graphic out and then puts the shell
     * over the same rectangle, which is what keeps the hit area and the visible
     * field in agreement.
     */
    template<typename Output>
    class text_input final : public ui::widget {
        /// How the commit output is kept
        /**
         * Something to write into -- a string, a queue, a future -- belongs to
         * the call site, which has somewhere to keep it, so it is held by
         * reference exactly as `planet::widget::button` holds its own. A
         * callback is the other way round: a call site that wants a commit to
         * *do* something usually has nowhere to put the callable, the builder
         * that made the field being a temporary, so the field owns that one.
         */
        template<typename Q>
        struct deduce {
            static constexpr bool reference = true;
            using storage = Q &;
        };
        template<std::invocable<text::editor::value_type const &> Q>
        struct deduce<Q> {
            static constexpr bool reference = false;
            using storage = Q;
        };

        typename deduce<Output>::storage output_to;
        text::editor edits;
        float resting_z_layer = {};


      public:
        using constrained_type = widget::constrained_type;
        using editor_type = text::editor;
        using output_type = Output;
        using value_type = editor_type::value_type;


        text_input(std::string_view const n, output_type &o, value_type v = {})
        : widget{n}, output_to{o}, edits{std::move(v)} {}
        text_input(std::string_view const n, output_type &&o, value_type v = {})
            requires(not deduce<output_type>::reference)
        : widget{n}, output_to{std::move(o)}, edits{std::move(v)} {}

        text_input(text_input &&t)
            requires deduce<output_type>::reference
        : widget{std::move(t)},
          output_to{t.output_to},
          edits{std::move(t.edits)},
          resting_z_layer{t.resting_z_layer},
          screen{std::move(t.screen)},
          ward{t.ward},
          editing_changed{std::move(t.editing_changed)},
          clicked{std::move(t.clicked)} {
            if (has_baseplate()) { response.post(behaviour()); }
        }
        text_input(text_input &&t)
            requires(not deduce<output_type>::reference)
        : widget{std::move(t)},
          output_to{std::move(t.output_to)},
          edits{std::move(t.edits)},
          resting_z_layer{t.resting_z_layer},
          screen{std::move(t.screen)},
          ward{t.ward},
          editing_changed{std::move(t.editing_changed)},
          clicked{std::move(t.clicked)} {
            if (has_baseplate()) { response.post(behaviour()); }
        }


        /// ### Attributes

        /// #### The screen that ends an edit
        /**
         * Live only while an edit runs, and placed relative to the field each
         * time one begins.
         */
        ui::screen screen{"planet::widget::text_input dismissal"};

        /// #### How far above the field the dismissal screen is placed
        /**
         * The screen has to cover everything the click that ends an edit could
         * otherwise land on, so it is lifted **relative to the field's own z
         * layer**: what layer the field itself sits at is not knowable from in
         * here. A field inside a `b5::ui::pop_over` starts above the modal's
         * own screen, and a fixed layer would put the dismissal screen -- and,
         * worse, the lifted field -- underneath the modal they belong to, so
         * the click would close the modal instead of ending the edit.
         *
         * The default clears the containment depths a panel around a field is
         * likely to produce. Raise it for a field whose siblings nest more
         * deeply than that; anything under the pointer that is still above the
         * screen takes the click instead, and the edit does not end.
         */
        float screen_z_lift = 10.0f;

        /// #### Asynchronous push warden
        felspar::io::warden *ward = nullptr;
        /**
         * When set, a commit resumes the consumer asynchronously through this
         * warden. Use this when the commit might destroy the field -- a call
         * site that rebuilds the widget tree the field lives in, for example,
         * would otherwise destroy the coroutine part way through doing the
         * writing. When left `nullptr` the commit pushes synchronously.
         */

        /// #### Edit state changes
        std::function<void(bool editing)> editing_changed;
        /**
         * Called with `true` when an edit begins and `false` when one ends,
         * however it ends. This is the seam a concrete widget binds the
         * platform's text input to -- switching it on raises the on-screen
         * keyboard on mobile -- keeping this widget free of any of that.
         */

        /// #### Where a click on the field landed
        std::function<void(affine::point2d)> clicked;
        /**
         * Called with the location of every left click that reaches the field,
         * the one that begins an edit included -- clicking into the middle of
         * the text starts editing there rather than at the end of it. The mouse
         * is not captured, so a click that reaches the field really was over
         * it.
         *
         * The shell makes nothing of the point. Turning it into a position in
         * the text means measuring glyphs, and only the presentation has a
         * font; it hands whatever it works out back through
         * `editor().set_cursor`, which snaps and clamps so a mis-measurement
         * cannot leave the caret somewhere illegal.
         */


        /// ### Observable state

        /// #### The editing state machine
        /**
         * The text and the buffer being typed, along with anything else that is
         * about the text rather than about routing -- a caret, a selection.
         * None of it is on the shell.
         */
        editor_type &editor() noexcept { return edits; }
        editor_type const &editor() const noexcept { return edits; }

        /// #### Whether an edit is running
        /**
         * The editor's, passed through because the presentation asks it every
         * frame to decide how to draw.
         */
        bool is_editing() const noexcept { return edits.is_editing(); }


        /// ### Attach to a baseplate
        /**
         * The dismissal screen is a widget in its own right, so it has to be
         * added to the baseplate alongside the field. It only joins the
         * routing for the frames in which it is drawn, which is only while an
         * edit runs.
         */
        void add_to(ui::baseplate &bp, ui::panel &parent) override {
            widget::add_to(bp, parent);
            screen.add_to(bp);
        }
        /**
         * Overriding one `add_to` hides the rest, and `add_to(widget &)` is
         * how a field is attached above a modal's screen -- which is the case
         * the dismissal screen most needs to get right.
         */
        using widget::add_to;


      private:
        /**
         * How a committed value reaches the call site, matching the mechanisms
         * `planet::widget::button` accepts for its own output.
         */
        template<typename Q>
        struct commit {
            static void write(text_input *self) {
                self->output_to = self->edits.value();
            }
        };
        template<typename R>
        struct commit<queue::pmc<R>> {
            static void write(text_input *self) {
                if (self->ward) {
                    self->output_to.push(*self->ward, self->edits.value());
                } else {
                    self->output_to.push(self->edits.value());
                }
            }
        };
        template<typename R>
        struct commit<queue::psc<R>> {
            static void write(text_input *self) {
                if (self->ward) {
                    self->output_to.push(*self->ward, self->edits.value());
                } else {
                    self->output_to.push(self->edits.value());
                }
            }
        };
        template<typename R>
        struct commit<felspar::coro::future<R>> {
            static void write(text_input *self) {
                self->output_to.set_value(self->edits.value());
            }
        };
        template<std::invocable<std::string const &> Q>
        struct commit<Q> {
            static void write(text_input *self) {
                self->output_to(self->edits.value());
            }
        };


        void begin_edit(affine::point2d const at) {
            edits.begin();
            /**
             * Place the dismissal screen over everything the field covers, and
             * then the field back over the screen. Both moves are relative to
             * where the field already is, so a field in a modal lifts within
             * its modal rather than out from under it.
             *
             * The lift is what tells a click on the field from a click
             * anywhere else: the mouse is not captured, so both are routed by
             * position, and without the field above the screen every click
             * would stop at the screen and end the edit.
             */
            screen.static_z_layer = z_layer() + screen_z_lift;
            resting_z_layer = std::exchange(
                    static_z_layer, static_z_layer + screen_z_lift + 1.0f);
            /// The keyboard kinds only -- see the class comment
            hard_focus_on({.mouse = false, .scroll = false});
            /**
             * `begin()` leaves the caret at the end of the value; where the
             * click landed is where it belongs. This is before the edit-state
             * callback so that whatever that tells about the caret is already
             * the answer, rather than one corrected on the next layout.
             */
            if (clicked) { clicked(at); }
            if (editing_changed) { editing_changed(true); }
        }

        void end_edit(bool const keep) {
            /**
             * The editor has the last word on whether what was typed may be
             * kept: a value its filter refuses puts the previous one back
             * instead. The edit still ends -- the focus goes back and the
             * platform's keyboard is put away either way -- but nothing is
             * written out, the call site already holding the value that is
             * being restored.
             */
            bool kept = false;
            if (keep) {
                kept = edits.commit();
            } else {
                edits.cancel();
            }
            static_z_layer = resting_z_layer;
            hard_focus_off();
            if (editing_changed) { editing_changed(false); }
            /**
             * The write is last because it is the only step that can reach
             * outside the widget, and a call site is allowed to react to a
             * commit by destroying the field. By then everything this widget
             * needs to touch has already been settled.
             */
            if (kept) { commit<output_type>::write(this); }
        }

        /// Act on what the editor made of an event it was handed
        void acted_on(text::outcome const o) {
            switch (o) {
            case text::outcome::commit: end_edit(true); break;
            case text::outcome::cancel: end_edit(false); break;
            case text::outcome::ignored:
            case text::outcome::changed: break;
            }
        }


        felspar::coro::task<void> activation() {
            auto mouse = widget::events.mouse.values();
            while (true) {
                auto const c = events::is_mouse_click(co_await mouse.next());
                if (not c or c->button != events::button::left) { continue; }
                /**
                 * The mouse is not captured, so a click that gets here is one
                 * the routing put here: it was over the field. One that was not
                 * went to the dismissal screen instead and ended the edit. The
                 * field still never asks where the click was -- that would mean
                 * duplicating layout state in an event coroutine -- the
                 * layering answers it, and the location only travels on to the
                 * presentation, which is measuring within a rectangle it laid
                 * out itself rather than making a routing decision.
                 */
                if (not is_editing()) {
                    begin_edit(c->location);
                } else if (clicked) {
                    clicked(c->location);
                }
            }
        }

        felspar::coro::task<void> dismissal() {
            auto clicks = events::identify_clicks(screen.events.mouse.stream());
            while (auto const c = co_await clicks.next()) {
                if (is_editing()) { end_edit(true); }
            }
        }

        felspar::coro::task<void> typing() {
            auto typed = widget::events.text.values();
            while (true) {
                auto const t = co_await typed.next();
                acted_on(edits.handle(t));
            }
        }

        felspar::coro::task<void> control_keys() {
            auto keys = widget::events.key.values();
            while (true) {
                auto const k = co_await keys.next();
                acted_on(edits.handle(k));
            }
        }


      protected:
        void do_draw() override {
            /**
             * A widget only joins the frame's live list, and so the event
             * routing, by drawing itself. Drawing the screen only while an
             * edit runs is what makes it dismiss during one and be invisible to
             * routing the rest of the time.
             */
            if (is_editing()) { screen.draw(); }
        }
        constrained_type do_reflow(
                reflow_parameters const &,
                constrained_type const &ex) override {
            return ex;
        }
        affine::rectangle2d do_move_sub_elements(
                reflow_parameters const &,
                affine::rectangle2d const &r) override {
            return r;
        }


        felspar::coro::task<void> behaviour() override {
            /**
             * Each queue gets its own coroutine so that all of them have a
             * consumer for the whole life of the behaviour. A single loop
             * could only ever be blocked on one of them, and delivery tests
             * `consumer_count()`, so the ones it was not waiting on would
             * silently fall through to whatever the field covers.
             */
            felspar::coro::starter<> streams;
            streams.post(activation());
            streams.post(dismissal());
            streams.post(typing());
            streams.post(control_keys());
            co_await streams.wait_for_all();
        }
    };


}
