#pragma once


#include <planet/queue/concepts.hpp>
#include <planet/queue/pmc.hpp>
#include <planet/queue/psc.hpp>
#include <planet/ui/drawable.hpp>
#include <planet/ui/widget.hpp>

#include <felspar/coro/future.hpp>


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
          graphic{std::move(b.graphic)} {
            if (has_baseplate()) { response.post(behaviour()); }
        }


        Texture graphic;


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
                    self->output_to.push(self->press_value);
                }
            }
        };
        template<typename R>
        struct handle<queue::psc<R>> {
            static felspar::coro::task<void> press(button *self, auto clicks) {
                while (true) {
                    auto click = co_await clicks.next();
                    self->output_to.push(self->press_value);
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

        template<queue::push_void_queue Q>
        struct handle<Q> {
            static constexpr bool reference = true;
            using storage = Q &;
            static felspar::coro::task<void> press(button *self, auto clicks) {
                while (true) {
                    auto click = co_await clicks.next();
                    self->output_to.push();
                }
            }
        };
        template<typename R>
        struct handle<felspar::coro::future<R>> {
            static constexpr bool reference = true;
            using storage = felspar::coro::future<R> &;
            static felspar::coro::task<void> press(button *self, auto clicks) {
                co_await clicks.next();
                self->output_to.set_value();
            }
        };
        template<std::invocable<> Q>
        struct handle<Q> {
            static constexpr bool reference = false;
            using storage = Q;
            static felspar::coro::task<void> press(button *self, auto clicks) {
                while (true) {
                    auto click = co_await clicks.next();
                    self->output_to();
                }
            }
        };
        template<std::invocable<widget &> Q>
        struct handle<Q> {
            static constexpr bool reference = false;
            using storage = Q;
            static felspar::coro::task<void> press(button *self, auto clicks) {
                while (true) {
                    auto click = co_await clicks.next();
                    self->output_to(*self);
                }
            }
        };


        typename handle<std::decay_t<Output>>::storage output_to;


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
            requires handle<output_type>::reference
        : widget{std::move(b)},
          output_to{b.output_to},
          graphic{std::move(b.graphic)} {
            if (has_baseplate()) { response.post(behaviour()); }
        }
        button(button &&b)
            requires(not handle<output_type>::reference)
        : widget{std::move(b)},
          output_to{std::move(b.output_to)},
          graphic{std::move(b.graphic)} {
            if (has_baseplate()) { response.post(behaviour()); }
        }


        Texture graphic;


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


    template<ui::drawable G, typename Q>
    button(std::string_view, G, Q) -> button<void, G, Q>;
    template<ui::drawable G, typename Q, typename R>
    button(std::string_view, G, Q, R) -> button<std::decay_t<R>, G, Q>;


}
