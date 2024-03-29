#pragma once


#include <planet/affine/extents2d.hpp>


namespace planet::affine {


    /// ## Axis aligned 2D rectangle
    struct rectangle2d {
        point2d top_left{};
        extents2d extents{};


        /// ###  Construction
        constexpr rectangle2d() {}
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

        bool contains(point2d const p) const {
            return p.x() >= left() and p.x() <= right() and p.y() >= top()
                    and p.y() <= bottom();
        }
        bool contains(rectangle2d const r) const {
            return contains(r.top_left) and contains(r.bottom_right());
        }

        friend bool operator==(rectangle2d const &l, rectangle2d const &r) =
                default;


        /// ### Move the rectangle
        friend rectangle2d operator+(rectangle2d const r, point2d const p) {
            return {r.top_left + p, r.extents};
        }
    };


}
