#pragma once


#include <planet/ui/draggable.hpp>


namespace planet::ui {


    /// ## Range/slide control
    template<typename Background, typename Draggable>
    class range : public widget, public drop_target {
      public:
        range(Background bg,
              Draggable ctrl,
              planet::ui::constrained1d<float> const &position)
        : widget{"planet::ui::range<>"},
          background{std::move(bg)},
          slider{std::move(ctrl)},
          slider_position{position} {
            slider.offset = {fully_constrained, fully_constrained};
        }
        range(std::string_view const n,
              Background bg,
              Draggable ctrl,
              planet::ui::constrained1d<float> const &position)
        : widget{n},
          background{std::move(bg)},
          slider{std::move(ctrl)},
          slider_position{position} {
            slider.offset = {fully_constrained, fully_constrained};
        }


        Background background;
        Draggable slider;
        planet::ui::constrained1d<float> slider_position = {};


        using constrained_type = widget::constrained_type;


        using widget::add_to;
        void add_to(ui::baseplate &bp, ui::panel &parent) override {
            widget::add_to(bp, parent);
            slider.target = this;
            slider.z_layer = widget::z_layer + 1;
            slider.add_to(bp, widget::panel);
        }


      private:
        static constexpr typename constrained_type::axis_constrained_type
                fully_constrained = {0, 0, 0};

        constrained_type do_reflow(constrained_type const &constraint) override {
            auto const bg = background.reflow(constraint);
            auto const s = slider.reflow(bg);
            slider.offset.max(bg.max() - s.max());
            return bg;
        }

        affine::rectangle2d
                do_move_sub_elements(affine::rectangle2d const &r) override {
            slider.move_to(
                    {r.top_left + slider.offset.position(),
                     slider.constraints().extents()});
            return background.move_to(r);
        }

        felspar::coro::task<void> behaviour() override { co_return; }

        constrained_type drop(constrained_type const &offset) override {
            return offset;
        }
    };


}
