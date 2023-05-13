#pragma once


#include <planet/ui/widget.hpp>


namespace planet::ui {


    template<typename Renderer, typename Return, typename Texture>
    class button : public planet::ui::widget<Renderer> {
        using superclass = planet::ui::widget<Renderer>;
        Return press_value;
        felspar::coro::bus<Return> &output_to;

      public:
        using constrained_type = typename superclass::constrained_type;


        explicit button(felspar::coro::bus<Return> &o, Return v)
        : superclass{"planet::sdl::ui::button"},
          press_value{std::move(v)},
          output_to{o} {}
        explicit button(
                std::string_view const n,
                felspar::coro::bus<Return> &o,
                Return v)
        : superclass{n}, press_value{std::move(v)}, output_to{o} {}
        explicit button(Texture g, felspar::coro::bus<Return> &o, Return v)
        : superclass{"planet::sdl::ui::button"},
          press_value{std::move(v)},
          output_to{o},
          graphic{std::move(g)} {}
        explicit button(
                std::string_view const n,
                Texture g,
                felspar::coro::bus<Return> &o,
                Return v)
        : superclass{n},
          press_value{std::move(v)},
          output_to{o},
          graphic{std::move(g)} {}

        Texture graphic;

      private:
        constrained_type do_reflow(constrained_type const &ex) override {
            return graphic.reflow(ex);
        }
        void do_move_sub_elements(affine::rectangle2d const &r) override {
            graphic.move_to(r);
        }

        felspar::coro::task<void> behaviour() override {
            for (auto clicks = events::identify_clicks(
                         superclass::baseplate->mouse_settings,
                         superclass::events.mouse.stream());
                 auto click = co_await clicks.next();) {
                output_to.push(press_value);
            }
        }
    };


}
