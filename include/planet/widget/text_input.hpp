#pragma once


#include <planet/queue/psc.hpp>
#include <planet/ui/drawable.hpp>
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
     * ```
     *
     * An edit holds the hard focus for as long as it runs, which puts the
     * field at the top of the baseplate's delivery stack so typing cannot leak
     * into whatever key controls sit beneath it.
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
          graphic{std::move(t.graphic)},
          ward{t.ward},
          editing_changed{std::move(t.editing_changed)} {
            if (has_baseplate()) { response.post(behaviour()); }
        }


        /// ### Attributes

        Texture graphic;

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
            hard_focus_on();
            if (editing_changed) { editing_changed(true); }
        }

        void end_edit(bool const keep) {
            if (not keep) { shown = std::move(before_edit); }
            before_edit.clear();
            editing = false;
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
            auto clicks =
                    events::identify_clicks(widget::events.mouse.stream());
            while (auto const c = co_await clicks.next()) {
                if (c->button == events::button::left and not editing) {
                    begin_edit();
                }
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
        void do_draw() override { graphic.draw(); }
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
             * Each of the three kinds gets its own coroutine so that all three
             * queues have a consumer for the whole life of the behaviour. A
             * single loop could only ever be blocked on one of them, and
             * delivery tests `consumer_count()`, so the two it was not waiting
             * on would silently fall through to whatever the field covers.
             */
            felspar::coro::starter<> streams;
            streams.post(activation());
            streams.post(typing());
            streams.post(control_keys());
            co_await streams.wait_for_all();
        }
    };


}
