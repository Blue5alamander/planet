#pragma once


#include <planet/affine/matrix2d.hpp>


namespace planet::affine {


    /// ## 2D Affine transform into and out of a co-ordinate space
    class transform2d final {
        matrix2d in, out;

      public:
        /// ### Reflect the y-axis, so up is down
        transform2d &reflect_y() {
            in = matrix2d::reflect_y() * in;
            out = out * matrix2d::reflect_y();
            return *this;
        }
        /// ### Scale up by the provided factor
        transform2d &scale(float const factor) {
            in = matrix2d::scale(factor) * in;
            out = out * matrix2d::scale(1.0f / factor);
            return *this;
        }
        /// ### Translate by the provided amount
        transform2d &translate(point2d const by) {
            in = matrix2d::translate(by) * in;
            out = out * matrix2d::translate(-by);
            return *this;
        }
        /// ### Rotate by the requested number of turns. One turn is τ radians
        transform2d &rotate(float const turns) {
            in = matrix2d::rotate(turns) * in;
            out = out * matrix2d::rotate(-turns);
            return *this;
        }

        /// ### Transform points into and out of co-ordinate space
        auto const &into() const { return in; }
        auto const &outof() const { return out; }
        point2d into(point2d const p) const { return in * p; }
        point2d outof(point2d const p) const { return out * p; }
    };


}
