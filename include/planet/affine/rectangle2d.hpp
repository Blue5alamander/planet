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
        constexpr point2d top_right() const noexcept {
            return {right(), top()};
        }
        constexpr point2d bottom_left() const noexcept {
            return {left(), bottom()};
        }
        constexpr point2d bottom_right() const noexcept {
            return top_left + extents;
        }

        constexpr float top() const noexcept { return top_left.y(); }
        constexpr float left() const noexcept { return top_left.x(); }
        constexpr float bottom() const noexcept {
            return top() + extents.height;
        }
        constexpr float right() const noexcept {
            return left() + extents.width;
        }

        constexpr bool contains(point2d const p) const {
            return p.x() >= left() and p.x() <= right() and p.y() >= top()
                    and p.y() <= bottom();
        }
        constexpr bool contains(rectangle2d const r) const {
            return contains(r.top_left) and contains(r.bottom_right());
        }

        friend constexpr bool operator==(
                rectangle2d const &l, rectangle2d const &r) = default;


        /// ### Move the rectangle
        friend constexpr rectangle2d
                operator+(rectangle2d const r, point2d const p) {
            return {r.top_left + p, r.extents};
        }
    };


}
