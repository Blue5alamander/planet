#pragma once


#include <planet/affine/extents2d.hpp>


namespace planet::affine {


    /// ## Axis aligned 2D rectangle
    struct rectangle2d {
        point2d top_left;
        extents2d extents;

        constexpr rectangle2d(point2d const tl, extents2d const ex)
        : top_left{tl}, extents{ex} {}
        constexpr rectangle2d(point2d const tl, point2d const br)
        : top_left{tl}, extents{br - tl} {}

        /// ### Determine position
        point2d bottom_right() const noexcept { return top_left + extents; }
        float top() const noexcept { return top_left.y(); }
        float left() const noexcept { return top_left.x(); }
        float bottom() const noexcept { return top() + extents.height; }
        float right() const noexcept { return left() + extents.width; }

        /// ### Move the rectangle
        friend rectangle2d operator+(rectangle2d const r, point2d const p) {
            return {r.top_left + p, r.extents};
        }
    };


}
