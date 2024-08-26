#pragma once


#include <planet/ui/draggable.hpp>


namespace planet::ui {


    /// ## Range/slide control
    template<typename Background, typename Draggable>
    class range final : public widget, public drop_target {
      public:
        range(Background bg,
              Draggable ctrl,
              planet::ui::constrained1d<float> const &position)
        : range{"planet::ui::range<>", std::move(bg), std::move(ctrl),
                position} {}
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
        /// ### Slider position mapping
        /// #### The original range that we're aiming for'
        constrained1d<float> slider_position = {};


        using constrained_type = widget::constrained_type;


        using widget::add_to;
        void add_to(ui::baseplate &bp, ui::panel &parent) override {
            widget::add_to(bp, parent);
            slider.target = this;
            slider.z_layer = widget::z_layer + 1;
            slider.add_to(bp, widget::panel);
        }


      protected:
        void do_draw() override {
            background.draw();
            slider.draw();
        }


      private:
        static constexpr typename constrained_type::axis_constrained_type
                fully_constrained = {0, 0, 0};

        constrained_type do_reflow(constrained_type const &constraint) override {
            auto const bg = background.reflow(constraint);
            auto const s = slider.reflow(bg);
            slider.offset.max(bg.max_extents() - s.max_extents());
            auto const offset_range =
                    slider.offset.max_position() - slider.offset.min_position();
            auto const offset_difference =
                    offset_range * slider_position.normalised_value();
            slider.offset.desire(
                    slider.offset.min_position() + offset_difference);
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
            constrained1d<float> const hypot{
                    std::hypot(
                            offset.width.value() - offset.width.min(),
                            offset.height.value() - offset.height.min()),
                    0,
                    std::hypot(
                            offset.width.max() - offset.width.min(),
                            offset.height.max() - offset.height.min())};
            slider_position.desire(hypot.remapped_to(slider_position));
            return offset;
        }
    };


}
