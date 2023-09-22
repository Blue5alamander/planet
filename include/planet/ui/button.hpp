#pragma once


#include <planet/queue/pmc.hpp>
#include <planet/ui/widget.hpp>


namespace planet::ui {


    template<
            typename Renderer,
            typename Return,
            typename Texture,
            typename Queue = queue::pmc<Return>>
    class button : public planet::ui::widget<Renderer> {
        using superclass = planet::ui::widget<Renderer>;
        Return press_value;
        Queue &output_to;


      public:
        using queue_type = Queue;
        using constrained_type = typename superclass::constrained_type;
        using superclass::name;
        using superclass::position;


        explicit button(queue_type &o, Return v)
        : superclass{"planet::sdl::ui::button"},
          press_value{std::move(v)},
          output_to{o} {}
        button(std::string_view const n, queue_type &o, Return v)
        : superclass{n}, press_value{std::move(v)}, output_to{o} {}
        button(Texture g, queue_type &o, Return v)
        : superclass{"planet::sdl::ui::button"},
          press_value{std::move(v)},
          output_to{o},
          graphic{std::move(g)} {}
        button(std::string_view const n, Texture g, queue_type &o, Return v)
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
