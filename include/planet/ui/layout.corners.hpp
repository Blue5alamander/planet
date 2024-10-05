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


        corners(affine::rectangle2d const &w,
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


        void draw() {
            top_left.draw();
            top_right.draw();
            bottom_left.draw();
            bottom_right.draw();
        }


      private:
        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &c) override {
            top_left.reflow(p, c);
            top_right.reflow(p, c);
            bottom_left.reflow(p, c);
            bottom_right.reflow(p, c);
            /// TODO Calculate a better min based on the corner constraints
            return constrained_type{c.max_extents()};
        }
        affine::rectangle2d move_sub_elements(
                reflow_parameters const &p,
                affine::rectangle2d const &r) override {
            auto const tlex = top_left.constraints().extents(),
                       trex = top_right.constraints().extents(),
                       blex = bottom_left.constraints().extents(),
                       brex = bottom_right.constraints().extents();

            top_left.move_to(p, {r.top_left, tlex});
            top_right.move_to(
                    p, {{r.extents.width - trex.width, r.top_left.y()}, trex});
            bottom_left.move_to(
                    p,
                    {{r.top_left.x(), r.extents.height - blex.height}, blex});
            bottom_right.move_to(
                    p,
                    {{r.extents.width - brex.width,
                      r.extents.height - brex.height},
                     brex});

            return r;
        }
    };


}
