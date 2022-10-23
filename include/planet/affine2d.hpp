#pragma once


#include <array>
#include <cmath>
#include <numbers>


namespace planet {


    /// Constant for radian conversions
    constexpr float τ = std::numbers::pi_v<float> * 2.0f;


    /// 2D point suitable for affine transformations
    struct point2d final {
        float xh = {}, yh = {}, h = {1.0f};

        point2d(float const x, float const y) : xh{x}, yh{y} {}
        point2d(float const x, float const y, float const h)
        : xh{x}, yh{y}, h{h} {}

        float x() const noexcept { return xh / h; }
        float y() const noexcept { return yh / h; }

        friend bool operator==(point2d l, point2d r) {
            return l.x() == r.x() and l.y() == r.y();
        }
    };


    /// 2D matrix used for affine transformations
    class matrix final {
        std::array<float, 9> m = {1, 0, 0,

                                  0, 1, 0,

                                  0, 0, 1};

        matrix(float a,
               float b,
               float c,
               float d,
               float e,
               float f,
               float g,
               float h,
               float i)
        : m{a, b, c, d, e, f, g, h, i} {}

      public:
        matrix() {}

        friend matrix operator*(matrix const &a, matrix const &b) {
            return matrix{
                    a.m[0] * b.m[0] + a.m[1] * b.m[3] + a.m[2] * b.m[6],
                    a.m[0] * b.m[1] + a.m[1] * b.m[4] + a.m[2] * b.m[7],
                    a.m[0] * b.m[2] + a.m[1] * b.m[5] + a.m[2] * b.m[8],

                    a.m[3] * b.m[0] + a.m[4] * b.m[3] + a.m[5] * b.m[6],
                    a.m[3] * b.m[1] + a.m[4] * b.m[4] + a.m[5] * b.m[7],
                    a.m[3] * b.m[2] + a.m[4] * b.m[5] + a.m[5] * b.m[8],

                    a.m[6] * b.m[0] + a.m[7] * b.m[3] + a.m[8] * b.m[6],
                    a.m[6] * b.m[1] + a.m[7] * b.m[4] + a.m[8] * b.m[7],
                    a.m[6] * b.m[2] + a.m[7] * b.m[5] + a.m[8] * b.m[8]};
        }
        friend point2d operator*(matrix const &a, point2d const p) {
            return {a.m[0] * p.xh + a.m[1] * p.yh + a.m[2] * p.h,
                    a.m[3] * p.xh + a.m[4] * p.yh + a.m[5] * p.h,
                    a.m[6] * p.xh + a.m[7] * p.yh + a.m[8] * p.h};
        }

        friend bool operator==(matrix const &, matrix const &) = default;

        static matrix rotate(float const turns) {
            float const r = turns * τ;
            return matrix{
                    std::cos(r),
                    -std::sin(r),
                    0,

                    std::sin(r),
                    std::cos(r),
                    0,

                    0,
                    0,
                    1};
        }
    };


    /// 2D Affine transform into and out of a co-ordinate space
    class transform final {
        matrix in, out;

      public:
        /// Rotate by the requested number of turns. One turn is τ radians
        transform &rotate(float const turns) {
            in = in * matrix::rotate(turns);
            out = matrix::rotate(-turns) * out;
            return *this;
        }

        point2d into(point2d const p) const { return in * p; }
        point2d outof(point2d const p) const { return out * p; }
    };


}
