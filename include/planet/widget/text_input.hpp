#pragma once


#include <planet/queue/psc.hpp>
#include <planet/ui/drawable.hpp>
#include <planet/ui/screen.hpp>
#include <planet/ui/widget.hpp>

#include <felspar/coro/future.hpp>


namespace planet::widget {


    /// ## Single line text input
    /**
     * A field that is activated with a click and then typed into. The value is
     * held as UTF-8 and, while an edit is running, is whatever has been typed
     * since the click -- this is the append-only field, with no caret and no
     * correction.
     *
     * ```mermaid
     * stateDiagram-v2
     *     [*] --> resting
     *     resting --> editing: left click
     *     editing --> resting: return_key -- write the buffer to the output
     *     editing --> resting: escape_key -- restore the previous value
     *     editing --> resting: a click the dismissal screen takes --
     *         write the buffer
     * ```
     *
     * An edit holds the hard focus for as long as it runs, which puts the
     * field at the top of the baseplate's delivery stack so typing cannot leak
     * into whatever key controls sit beneath it.
     *
     * Ending an edit with the mouse is the dismissal screen's job, following
     * the pattern `b5::ui::singleselect` uses for its open drop down. While an
     * edit runs the field draws a `planet::ui::screen` beneath itself and
     * lifts itself above it, so the screen is the topmost widget everywhere
     * the field is not. The hard focus means every click still arrives at the
     * field first, so it passes each one down to the screen, which ends the
     * edit. The screen is a hover boundary, so the click stops there: a click
     * that finishes an edit dismisses, it does not also press whatever was
     * underneath it.
     *
     * The widget is presentation agnostic: it holds no border, size, colour or
     * z layer and draws only the graphic it is handed. `is_editing()` and
     * `value()` are what the presentation layer selects on to decide how the
     * resting and editing states look.
     */
    template<ui::drawable Texture, typename Output>
    class text_input final : public ui::widget {
        Output &output_to;
        std::string shown, before_edit;
        bool editing = false;
        float resting_z_layer = {};


      public:
        using constrained_type = widget::constrained_type;
        using output_type = Output;
        using value_type = std::string;


        text_input(
                std::string_view const n,
                Texture g,
                output_type &o,
                value_type v = {})
        : widget{n}, output_to{o}, shown{std::move(v)}, graphic{std::move(g)} {}

        text_input(text_input &&t)
        : widget{std::move(t)},
          output_to{t.output_to},
          shown{std::move(t.shown)},
          before_edit{std::move(t.before_edit)},
          editing{t.editing},
          resting_z_layer{t.resting_z_layer},
          graphic{std::move(t.graphic)},
          screen{std::move(t.screen)},
          ward{t.ward},
          editing_changed{std::move(t.editing_changed)} {
            if (has_baseplate()) { response.post(behaviour()); }
        }


        /// ### Attributes

        Texture graphic;

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


        /// ### Observable state

        /// #### Whether an edit is running
        bool is_editing() const noexcept { return editing; }

        /// #### The text to show
        /**
         * The value at rest, and the live buffer while an edit is running.
         */
        std::string const &value() const noexcept { return shown; }


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
                self->output_to = self->shown;
            }
        };
        template<typename R>
        struct commit<queue::pmc<R>> {
            static void write(text_input *self) {
                if (self->ward) {
                    self->output_to.push(*self->ward, self->shown);
                } else {
                    self->output_to.push(self->shown);
                }
            }
        };
        template<typename R>
        struct commit<queue::psc<R>> {
            static void write(text_input *self) {
                if (self->ward) {
                    self->output_to.push(*self->ward, self->shown);
                } else {
                    self->output_to.push(self->shown);
                }
            }
        };
        template<typename R>
        struct commit<felspar::coro::future<R>> {
            static void write(text_input *self) {
                self->output_to.set_value(self->shown);
            }
        };
        template<std::invocable<std::string const &> Q>
        struct commit<Q> {
            static void write(text_input *self) {
                self->output_to(self->shown);
            }
        };


        void begin_edit() {
            before_edit = std::exchange(shown, {});
            editing = true;
            /**
             * Place the dismissal screen over everything the field covers, and
             * then the field back over the screen. Both moves are relative to
             * where the field already is, so a field in a modal lifts within
             * its modal rather than out from under it.
             *
             * The hard focus is what actually brings the events here, but
             * without the lift the screen would cover the field for hover as
             * well, and a click on the field would be indistinguishable from
             * one anywhere else the moment the hard focus is given up.
             */
            screen.static_z_layer = z_layer() + screen_z_lift;
            resting_z_layer = std::exchange(
                    static_z_layer, static_z_layer + screen_z_lift + 1.0f);
            hard_focus_on();
            if (editing_changed) { editing_changed(true); }
        }

        void end_edit(bool const keep) {
            if (not keep) { shown = std::move(before_edit); }
            before_edit.clear();
            editing = false;
            static_z_layer = resting_z_layer;
            hard_focus_off();
            if (editing_changed) { editing_changed(false); }
            /**
             * The write is last because it is the only step that can reach
             * outside the widget, and a call site is allowed to react to a
             * commit by destroying the field. By then everything this widget
             * needs to touch has already been settled.
             */
            if (keep) { commit<output_type>::write(this); }
        }


        felspar::coro::task<void> activation() {
            auto mouse = widget::events.mouse.values();
            while (true) {
                auto const m = co_await mouse.next();
                if (editing) {
                    /**
                     * The hard focus puts the field on the top of the delivery
                     * stack whatever the pointer is over, so while an edit runs
                     * every mouse event arrives here first. The field wants
                     * none of them, so it hands each straight down to the
                     * dismissal screen, which is the next widget in the stack
                     * and is what decides an edit is over. The field never asks
                     * where the click was: that would mean duplicating layout
                     * state in an event coroutine, and the layering already
                     * answers it.
                     */
                    widget::forward(m);
                } else if (auto const c = events::is_mouse_click(m);
                           c and c->button == events::button::left) {
                    begin_edit();
                }
            }
        }

        felspar::coro::task<void> dismissal() {
            auto clicks = events::identify_clicks(screen.events.mouse.stream());
            while (auto const c = co_await clicks.next()) {
                if (editing) { end_edit(true); }
            }
        }

        felspar::coro::task<void> typing() {
            auto text = widget::events.text.values();
            while (true) {
                auto const t = co_await text.next();
                if (editing) { shown += t.utf8; }
            }
        }

        felspar::coro::task<void> control_keys() {
            auto keys = widget::events.key.values();
            while (true) {
                auto const k = co_await keys.next();
                if (editing and k.action == events::action::down) {
                    if (k.scancode == events::scancode::return_key) {
                        end_edit(true);
                    } else if (k.scancode == events::scancode::escape_key) {
                        end_edit(false);
                    }
                }
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
            if (editing) { screen.draw(); }
            graphic.draw();
        }
        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &ex) override {
            return graphic.reflow(p, ex);
        }
        affine::rectangle2d do_move_sub_elements(
                reflow_parameters const &p,
                affine::rectangle2d const &r) override {
            return graphic.move_to(p, r);
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
