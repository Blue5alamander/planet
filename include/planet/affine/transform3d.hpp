#pragma once


#include <planet/affine/matrix3d.hpp>
#include <planet/serialise/forward.hpp>


namespace planet::affine {


    /// ## 3D Affine transform into and out of a co-ordinate space
    class transform3d final {
        matrix3d in, out;

        transform3d(
                std::array<std::array<float, 4>, 4> const &i,
                std::array<std::array<float, 4>, 4> const &o)
        : in{i}, out{o} {}


      public:
        transform3d() = default;
        transform3d(matrix3d const &i, matrix3d const &o) : in{i}, out{o} {}


        /// ### Transformation
        /**
         * Rotations are given in terms of "turns". A fuil turn is 360°.
         */
        transform3d &translate(point3d const &);
        transform3d &rotate_x(float);
        transform3d &rotate_y(float);
        transform3d &rotate_z(float);


        /// ### Perspective matrix
        /**
         * Creates a perspective projection matrix with its inverse.
         *
         * 1. `aspect` ratio of the screen
         * 2. `theta` is the FoV as a factor of 90 degrees
         *
         * The near plane is always at 1.0f, and the far plane is infinitely far
         * away.
         */
        static transform3d perspective(float aspect, float theta);


        /// ### Transform points into and out of co-ordinate space
        constexpr auto const &into() const noexcept { return in; }
        constexpr auto const &outof() const noexcept { return out; }
        constexpr point3d into(point3d const &p) const noexcept {
            return in * p;
        }
        constexpr point3d outof(point3d const &p) const noexcept {
            return out * p;
        }


        /// ### Queries
        friend bool
                operator==(transform3d const &, transform3d const &) = default;
    };


    /// ### Combine transforms
    transform3d operator*(transform3d const &, transform3d const &);


}
