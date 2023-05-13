#pragma once


#include <planet/ui/reflowable.hpp>


namespace planet::ui {


    /// ## UI element in each corner
    /**
     * UI elements can all be of different types and are drawn as far apart as
     * possible within the confines of the draw rectangle. The calculated extents
     * is the minimum extent which will fit all four elements without overlap.
     *
     * There is no support for padding.
     */
    template<typename TL, typename TR, typename BL, typename BR>
    struct corners final : public reflowable {
        using top_left_type = TL;
        using top_right_type = TR;
        using bottom_left_type = BL;
        using bottom_right_type = BR;

        explicit corners(
                affine::rectangle2d const &w,
                top_left_type tl,
                top_right_type tr,
                bottom_left_type bl,
                bottom_right_type br)
        : reflowable{"planet::ui::corners<>"},
          within{w},
          top_left{std::move(tl)},
          top_right{std::move(tr)},
          bottom_left{std::move(bl)},
          bottom_right{std::move(br)} {}

        affine::rectangle2d within;
        top_left_type top_left;
        top_right_type top_right;
        bottom_left_type bottom_left;
        bottom_right_type bottom_right;

        using constrained_type = planet::ui::reflowable::constrained_type;

        template<typename Renderer>
        void draw(Renderer &r) {
            top_left.draw(r);
            top_right.draw(r);
            bottom_left.draw(r);
            bottom_right.draw(r);
        }

      private:
        constrained_type do_reflow(constrained_type const &c) override {
            top_left.reflow(c);
            top_right.reflow(c);
            bottom_left.reflow(c);
            bottom_right.reflow(c);
            /// TODO Calculate a better min based on the corner constraints
            return constrained_type{c.max()};
        }
        void move_sub_elements(affine::rectangle2d const &r) override {
            auto const tlex = top_left.constraints().extents(),
                       trex = top_right.constraints().extents(),
                       blex = bottom_left.constraints().extents(),
                       brex = bottom_right.constraints().extents();

            top_left.move_to({r.top_left, tlex});
            top_right.move_to(
                    {{r.extents.width - trex.width, r.top_left.y()}, trex});
            bottom_left.move_to(
                    {{r.top_left.x(), r.extents.height - blex.height}, blex});
            bottom_right.move_to(
                    {{r.extents.width - brex.width,
                      r.extents.height - brex.height},
                     brex});
        }
    };


}
