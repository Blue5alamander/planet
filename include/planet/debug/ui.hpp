#pragma once


#include <planet/affine/rectangle2d.hpp>


namespace planet::debug {


    /// UI element
    struct fixed_element {
        affine::extents2d size;
        std::optional<affine::rectangle2d> moved_to;

        fixed_element(affine::extents2d const s) : size{s} {}

        affine::extents2d extents(affine::extents2d) const noexcept {
            return size;
        }
        void move_to(affine::rectangle2d const &r) { moved_to = r; }

        void draw_within(std::ostream &, affine::rectangle2d);
    };


}
