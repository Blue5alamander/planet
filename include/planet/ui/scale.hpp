#pragma once


#include <planet/affine/extents2d.hpp>
#include <planet/ui/constrained.hpp>


namespace planet::ui {


    /// Determines how scaling works
    enum scale : unsigned char {
        never = 0,
        shrink_x = 1,
        expand_x = 2,
        shrink_y = 4,
        expand_y = 8,
        lock_aspect = 128 + 1 + 2 + 4 + 8,
    };
    inline scale operator|(scale l, scale r) {
        return static_cast<scale>(
                static_cast<unsigned char>(l)
                bitor static_cast<unsigned char>(r));
    }

    /// Calculate a new extent based on a scaling characteristic
    affine::extents2d
            scaling(affine::extents2d size, affine::extents2d bounds, scale);
    constrained2d<float>
            scaling(affine::extents2d size, constrained2d<float> bounds, scale);


}
