#pragma once


#include <planet/queue/pmc.hpp>
#include <planet/ui/drawable.hpp>
#include <planet/ui/widget.hpp>

#include <felspar/coro/future.hpp>


namespace planet::ui {


    /// ## Button
    /**
     * A UI element that sets a particular value when a mouse click is detected
     * inside its area. The button is customised with a graphical element. The
     * value of the mouse press is always the same, and is passed to the output.
     */
    template<typename Return, drawable Texture, typename Output>
    class button : public widget {
        Return press_value;
        Output &output_to;


      public:
        using value_type = Return;
        using output_type = Output;
        using constrained_type = widget::constrained_type;
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

        Texture graphic;


      private:
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
        struct handle<felspar::coro::future<R>> {
            static felspar::coro::task<void> press(button *self, auto clicks) {
                co_await clicks.next();
                self->output_to.set_value(self->press_value);
            }
        };


      protected:
        constrained_type do_reflow(constrained_type const &ex) override {
            return graphic.reflow(ex);
        }
        void do_move_sub_elements(affine::rectangle2d const &r) override {
            graphic.move_to(r);
        }


        felspar::coro::task<void> behaviour() override {
            return handle<output_type>::press(
                    this,
                    events::identify_clicks(
                            widget::baseplate->mouse_settings,
                            widget::events.mouse.stream()));
        }
    };

    template<drawable Texture, typename Output>
    class button<void, Texture, Output> : public widget {
        Output &output_to;


      public:
        using value_type = void;
        using output_type = Output;
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
        button(std::string_view const n, Texture g, output_type &o)
        : widget{n}, output_to{o}, graphic{std::move(g)} {}

        Texture graphic;


      private:
        template<typename Q>
        struct handle {
            static felspar::coro::task<void> press(button *self, auto clicks) {
                while (true) {
                    auto click = co_await clicks.next();
                    self->output_to.push();
                }
            }
        };
        template<typename R>
        struct handle<felspar::coro::future<R>> {
            static felspar::coro::task<void> press(button *self, auto clicks) {
                co_await clicks.next();
                self->output_to.set_value();
            }
        };


      protected:
        constrained_type do_reflow(constrained_type const &ex) override {
            return graphic.reflow(ex);
        }
        void do_move_sub_elements(affine::rectangle2d const &r) override {
            graphic.move_to(r);
        }


        felspar::coro::task<void> behaviour() override {
            return handle<output_type>::press(
                    this,
                    events::identify_clicks(
                            widget::baseplate->mouse_settings,
                            widget::events.mouse.stream()));
        }
    };


}
