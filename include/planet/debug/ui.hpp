#pragma once


#include <planet/affine/rectangle2d.hpp>


namespace planet::debug {


    /// UI element
    struct ui_element {
        affine::extents2d size;

        ui_element(affine::extents2d const s) : size{s} {}

        affine::extents2d extents(affine::extents2d) const noexcept {
            return size;
        }

        void draw_within(std::ostream &, affine::rectangle);
    };


}
