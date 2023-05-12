#pragma once


#include <planet/ui/draggable.hpp>


namespace planet::ui {


    /// ## Range/slide control
    template<typename Renderer, typename Background, typename Draggable>
    class range : public widget<Renderer>, public drop_target {
      public:
        range(Background bg,
              Draggable ctrl,
              planet::ui::constrained1d<float> const &position)
        : widget<Renderer>{"planet::ui::range<>"},
          background{std::move(bg)},
          slider{std::move(ctrl)},
          slider_position{position} {
            slider.offset = {fully_constrained, fully_constrained};
        }
        range(std::string_view const n,
              Background bg,
              Draggable ctrl,
              planet::ui::constrained1d<float> const &position)
        : widget<Renderer>{n},
          background{std::move(bg)},
          slider{std::move(ctrl)},
          slider_position{position} {
            slider.offset = {fully_constrained, fully_constrained};
        }

        Background background;
        Draggable slider;
        planet::ui::constrained1d<float> slider_position = {};

        using constrained_type =
                typename planet::ui::widget<Renderer>::constrained_type;

        using planet::ui::widget<Renderer>::add_to;
        void
                add_to(planet::ui::baseplate<Renderer> &bp,
                       planet::ui::panel &parent,
                       float z_layer = {}) override {
            planet::ui::widget<Renderer>::add_to(bp, parent, z_layer);
            slider.target = this;
            slider.add_to(bp, widget<Renderer>::panel, z_layer + 1);
        }


      protected:
        float px_offset = {};


      private:
        static constexpr typename constrained_type::axis_constrained_type
                fully_constrained = {0, 0, 0};

        constrained_type do_reflow(constrained_type const &constraint) override {
            auto const bg = background.reflow(constraint);
            auto const r = slider.reflow(bg);
            auto const slider_offset = affine::point2d{px_offset, 0};
            slider.move_to({slider_offset, r.extents()});
            return bg;
        }

        void do_move_sub_elements(affine::rectangle2d const &r) override {
            background.move_to(r);
            auto const slider_offset = affine::point2d{px_offset, 0};
            slider.move_to(
                    {r.top_left + slider_offset,
                     slider.constraints().extents()});
        }

        felspar::coro::task<void> behaviour() override { co_return; }

        constrained_type drop(constrained_type const &offset) override {
            px_offset += offset.width.value();
            return {fully_constrained, fully_constrained};
        }
    };


}
