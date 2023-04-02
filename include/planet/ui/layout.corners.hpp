#pragma once


#include <planet/affine2d.hpp>


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
    struct corners {
        using top_left_type = TL;
        using top_right_type = TR;
        using bottom_left_type = BL;
        using bottom_right_type = BR;

        affine::rectangle2d within;
        top_left_type top_left;
        top_right_type top_right;
        bottom_left_type bottom_left;
        bottom_right_type bottom_right;

        corners(affine::rectangle2d const &w,
                top_left_type tl,
                top_right_type tr,
                bottom_left_type bl,
                bottom_right_type br)
        : within{w},
          top_left{std::move(tl)},
          top_right{std::move(tr)},
          bottom_left{std::move(bl)},
          bottom_right{std::move(br)} {}

        affine::extents2d extents(affine::extents2d const outer) {
            auto const tl = top_left.extents(outer);
            auto const tr = top_right.extents(outer);
            auto const bl = bottom_left.extents(outer);
            auto const br = bottom_right.extents(outer);

            auto const width =
                    std::max(tl.width + tr.width, bl.width + br.width);
            auto const height =
                    std::max(tl.height + bl.height, tr.height + br.height);

            return {width, height};
        }

        template<typename Target>
        void draw_within(Target &t, affine::rectangle2d const bounds) {
            auto const tl_size = top_left.extents(bounds.extents);
            top_left.draw_within(t, {bounds.top_left, tl_size});

            auto const tr_size = top_right.extents(bounds.extents);
            top_right.draw_within(
                    t,
                    {{bounds.bottom_right().x() - tr_size.width,
                      bounds.top_left.y()},
                     tr_size});

            auto const bl_size = bottom_left.extents(bounds.extents);
            bottom_left.draw_within(
                    t,
                    {{bounds.top_left.x(),
                      bounds.bottom_right().y() - bl_size.height},
                     bl_size});

            auto const br_size = bottom_right.extents(bounds.extents);
            bottom_right.draw_within(
                    t,
                    {{bounds.bottom_right().x() - br_size.width,
                      bounds.bottom_right().y() - bl_size.height},
                     br_size});
        }
    };


}
