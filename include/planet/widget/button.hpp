#pragma once


#include <planet/queue/concepts.hpp>
#include <planet/queue/pmc.hpp>
#include <planet/queue/psc.hpp>
#include <planet/ui/drawable.hpp>
#include <planet/ui/widget.hpp>

#include <felspar/coro/future.hpp>
#include <felspar/io/warden.hpp>


namespace planet::widget {


    /// ## Button
    /**
     * A UI element that sets a particular value when a mouse click is detected
     * inside its area. The button is customised with a graphical element. The
     * value of the mouse press is always the same, and is passed to the output.
     */
    template<typename Return, ui::drawable Texture, typename Output>
    class button final : public ui::widget {
        Return press_value;
        Output &output_to;


      public:
        using constrained_type = widget::constrained_type;
        using output_type = Output;
        using value_type = Return;

        using widget::enable;
        using widget::name;
        using widget::position;


        explicit button(output_type &o, value_type v)
        : widget{"planet::sdl::ui::button"},
          press_value{std::move(v)},
          output_to{o} {}
        button(std::string_view const n, output_type &o, value_type v)
        : widget{n}, press_value{std::move(v)}, output_to{o} {}
        button(Texture g, output_type &o, value_type v)
        : widget{"planet::sdl::ui::button"},
          press_value{std::move(v)},
          output_to{o},
          graphic{std::move(g)} {}
        button(std::string_view const n, Texture g, output_type &o, value_type v)
        : widget{n},
          press_value{std::move(v)},
          output_to{o},
          graphic{std::move(g)} {}

        button(button &&b)
        : widget{std::move(b)},
          press_value{std::move(b.press_value)},
          output_to{b.output_to},
          graphic{std::move(b.graphic)},
          ward{b.ward} {
            if (has_baseplate()) { response.post(behaviour()); }
        }


        /// ### Attributes

        Texture graphic;

        /// #### Asynchronous push warden
        felspar::io::warden *ward = nullptr;
        /**
         * When set, a button press resumes the consumer asynchronously through
         * this warden. Use this when the press might destroy the button (for
         * example a button that dismisses the screen it lives on). When left
         * `nullptr` the press pushes synchronously.
         */


      private:
        /**
         * TODO It's likely much better to use concepts to direct these
         * overloads in the right direction. That way we also get to cut down on
         * the number of includes we need as we don't need to name the types
         * directly.
         */
        template<typename Q>
        struct handle {
            static felspar::coro::task<void> press(button *self, auto clicks) {
                while (true) {
                    co_await clicks.next();
                    self->output_to = self->press_value;
                }
            }
        };
        template<typename R>
        struct handle<queue::pmc<R>> {
            static felspar::coro::task<void> press(button *self, auto clicks) {
                while (true) {
                    auto click = co_await clicks.next();
                    if (self->ward) {
                        self->output_to.push(*self->ward, self->press_value);
                    } else {
                        self->output_to.push(self->press_value);
                    }
                }
            }
        };
        template<typename R>
        struct handle<queue::psc<R>> {
            static felspar::coro::task<void> press(button *self, auto clicks) {
                while (true) {
                    auto click = co_await clicks.next();
                    if (self->ward) {
                        self->output_to.push(*self->ward, self->press_value);
                    } else {
                        self->output_to.push(self->press_value);
                    }
                }
            }
        };
        template<typename R>
        struct handle<felspar::coro::future<R>> {
            static felspar::coro::task<void> press(button *self, auto clicks) {
                co_await clicks.next();
                self->output_to.set_value(self->press_value);
            }
        };


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


        FELSPAR_CORO_WRAPPER felspar::coro::task<void> behaviour() override {
            return handle<output_type>::press(
                    this,
                    events::identify_clicks(widget::events.mouse.stream()));
        }
    };

    template<ui::drawable Texture, typename Output>
    class button<void, Texture, Output> final : public ui::widget {
        template<typename Q>
        struct handle;

        template<typename Q>
        struct deduce {
            static constexpr bool reference = false;
            using storage = Q;
        };
        template<queue::push_void_queue Q>
        struct deduce<Q> {
            static constexpr bool reference = true;
            using storage = Q &;
        };
        template<typename R>
        struct deduce<felspar::coro::future<R>> {
            static constexpr bool reference = true;
            using storage = felspar::coro::future<R> &;
        };

        typename deduce<std::decay_t<Output>>::storage output_to;


      public:
        using value_type = void;
        using output_type = std::decay_t<Output>;
        using constrained_type = widget::constrained_type;
        using widget::name;
        using widget::position;


        explicit button(output_type &o)
        : widget{"planet::sdl::ui::button"}, output_to{o} {}
        button(std::string_view const n, output_type &o)
        : widget{n}, output_to{o} {}
        button(Texture g, output_type &o)
        : widget{"planet::sdl::ui::button"},
          output_to{o},
          graphic{std::move(g)} {}
        button(Texture g, output_type &&o)
        : widget{"planet::sdl::ui::button"},
          output_to{std::move(o)},
          graphic{std::move(g)} {}
        button(std::string_view const n, Texture g, output_type &o)
        : widget{n}, output_to{o}, graphic{std::move(g)} {}
        button(std::string_view const n, Texture g, output_type &&o)
        : widget{n}, output_to{std::move(o)}, graphic{std::move(g)} {}

        button(button &&b)
            requires deduce<output_type>::reference
        : widget{std::move(b)},
          output_to{b.output_to},
          graphic{std::move(b.graphic)},
          ward{b.ward} {
            if (has_baseplate()) { response.post(behaviour()); }
        }
        button(button &&b)
            requires(not deduce<output_type>::reference)
        : widget{std::move(b)},
          output_to{std::move(b.output_to)},
          graphic{std::move(b.graphic)},
          ward{b.ward} {
            if (has_baseplate()) { response.post(behaviour()); }
        }


        /// ### Attributes

        Texture graphic;

        /// #### Asynchronous push warden
        felspar::io::warden *ward = nullptr;
        /**
         * When set, a button press resumes the consumer asynchronously through
         * this warden. Use this when the press might destroy the button (for
         * example a button that dismisses the screen it lives on). When left
         * `nullptr` the press pushes synchronously.
         */


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
            return handle<output_type>::press(
                    this,
                    events::identify_clicks(widget::events.mouse.stream()));
        }
    };


    /// ### Specialisations for different deliveries of button clicks
    /**
     * `behaviour` turns the button's mouse events into a stream of clicks and
     * hands it to the `handle` for whatever the button was given to press
     * into, so what a press does is settled by the type of that output alone
     * and the button itself knows nothing about how the press is delivered.
     *
     * The constraints the specialisations are chosen by do not order against
     * each other, so an output must satisfy exactly one of them: a callable
     * taking `auto` is invocable both with the button and with the click, and
     * an output matching two is an ambiguity rather than a choice. A callback
     * therefore says what it wants by naming its parameter's type.
     *
     * A queue and a future are held by reference — that is what `deduce`
     * settles — so they must outlive the button, while a callback is owned by
     * it and moves with it.
     */

    /// #### A queue told that the button was pressed
    /**
     * The item type carries nothing, the press itself being the whole
     * message, so every press pushes an empty value. When a `ward` is set the
     * push goes through it, so a press that destroys the button — one
     * dismissing the screen the button lives on — resumes the consumer after
     * this coroutine has let go of it.
     */
    template<ui::drawable Texture, typename Output>
    template<queue::push_void_queue Q>
    struct button<void, Texture, Output>::handle<Q> {
        static felspar::coro::task<void> press(button *self, auto clicks) {
            while (true) {
                auto click = co_await clicks.next();
                if (self->ward) {
                    self->output_to.push(*self->ward);
                } else {
                    self->output_to.push();
                }
            }
        }
    };
    /// #### A future satisfied by the first press
    /**
     * A future can only be set once, so this is the one delivery that is not
     * a loop: the first click sets the value and the behaviour ends, leaving
     * later clicks on the button unanswered. It suits a button whose press is
     * awaited once — the answer to a question the coroutine that built the
     * button is sitting on — rather than one pressed over and over.
     */
    template<ui::drawable Texture, typename Output>
    template<std::same_as<felspar::coro::future<void>> Q>
    struct button<void, Texture, Output>::handle<Q> {
        static felspar::coro::task<void> press(button *self, auto clicks) {
            co_await clicks.next();
            self->output_to.set_value();
        }
    };
    /// #### A callback that only needs to know a press happened
    /**
     * The plainest delivery, and what most buttons are built with: the
     * callback is called with nothing on every press, and the click that
     * caused it is read and dropped. A callback wanting anything the event
     * carries takes the click instead — see below.
     */
    template<ui::drawable Texture, typename Output>
    template<std::invocable<> Q>
    struct button<void, Texture, Output>::handle<Q> {
        static felspar::coro::task<void> press(button *self, auto clicks) {
            while (true) {
                auto click = co_await clicks.next();
                self->output_to();
            }
        }
    };
    /// #### A callback taking the button it was pressed on
    /**
     * Called with the button itself on every press, so a callback can act on
     * the widget it belongs to — reading or changing what it draws, or
     * disabling it — rather than having to be handed an address that the
     * moves into a layout may since have invalidated. What it is given is
     * where the button has ended up, not where it was built.
     */
    template<ui::drawable Texture, typename Output>
    template<std::invocable<button<void, Texture, Output> &> Q>
    struct button<void, Texture, Output>::handle<Q> {
        static felspar::coro::task<void> press(button *self, auto clicks) {
            while (true) {
                auto click = co_await clicks.next();
                self->output_to(*self);
            }
        }
    };
    /// #### A callback taking the click that pressed the button
    /**
     * The press is handed the whole `events::click`, so a callback can tell a
     * shift-click from the plain kind — the modifiers held at the time are
     * already carried on the event, and the callbacks above simply drop them.
     * Match a combination by comparing against a constructed
     * `events::modifiers` value, the way key presses are matched.
     */
    template<ui::drawable Texture, typename Output>
    template<std::invocable<events::click> Q>
    struct button<void, Texture, Output>::handle<Q> {
        static felspar::coro::task<void> press(button *self, auto clicks) {
            while (auto click = co_await clicks.next()) {
                self->output_to(*click);
            }
        }
    };


    template<ui::drawable G, typename Q>
    button(std::string_view, G, Q) -> button<void, G, Q>;
    template<ui::drawable G, typename Q, typename R>
    button(std::string_view, G, Q, R) -> button<std::decay_t<R>, G, Q>;


}
