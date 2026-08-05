#pragma once


#include <planet/affine2d.hpp>


namespace planet::ui {


    /// ## Gravity direction
    /**
     * Describes where an element sits inside the space it has been given. The
     * two axes are independent of each other, and each is either pulled to one
     * side or left alone to centre.
     *
     * Because an axis carries a single direction, opposing pulls cannot be
     * written down -- `left | right` and `top | bottom` have no overload and so
     * fail to compile.
     *
     * ```cpp
     * gravity::top | gravity::left // the top left corner
     * gravity::top                 // the top edge, centred horizontally
     * gravity{}                    // centred on both axes
     * ```
     */
    struct gravity final {
        /// ### The pull along each axis
        enum class horizontal : unsigned char { centre, left, right };
        enum class vertical : unsigned char { centre, top, bottom };

        static constexpr horizontal left = horizontal::left;
        static constexpr horizontal right = horizontal::right;
        static constexpr vertical top = vertical::top;
        static constexpr vertical bottom = vertical::bottom;


        constexpr gravity() noexcept = default;
        constexpr gravity(horizontal const h) noexcept : horizontally{h} {}
        constexpr gravity(vertical const v) noexcept : vertically{v} {}
        constexpr gravity(horizontal const h, vertical const v) noexcept
        : horizontally{h}, vertically{v} {}


        horizontal horizontally = horizontal::centre;
        vertical vertically = vertical::centre;
    };


    /// ## Combining the axes
    constexpr gravity operator|(
            gravity::horizontal const h, gravity::vertical const v) noexcept {
        return {h, v};
    }
    constexpr gravity operator|(
            gravity::vertical const v, gravity::horizontal const h) noexcept {
        return {h, v};
    }


    /// ## Rectangle positioning
    /**
     * Calculate the extent within the outer extent that the inner will have
     * based on the gravity passed in
     */
    affine::rectangle2d
            within(gravity,
                   affine::rectangle2d const &outer,
                   affine::extents2d const &inner) noexcept;


}
