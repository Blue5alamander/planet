#pragma once


#include <array>
#include <cmath>
#include <numbers>


namespace planet::affine {


    /// Constant for radian conversions
    constexpr float τ = std::numbers::pi_v<float> * 2.0f;


    /// 2D point suitable for affine transformations
    struct point2d final {
        float xh = {}, yh = {}, h = {1.0f};

        constexpr point2d(float const x, float const y) : xh{x}, yh{y} {}
        constexpr point2d(float const x, float const y, float const h)
        : xh{x}, yh{y}, h{h} {}

        constexpr static point2d from_polar(float mag, float turns) {
            float const r = turns * τ;
            float const x = std::cos(r);
            float const y = std::sin(r);
            return {x, y, 1.0f / mag};
        }

        constexpr float x() const noexcept { return xh / h; }
        constexpr float x(float const n) noexcept { return xh = n * h; }
        constexpr float y() const noexcept { return yh / h; }
        constexpr float y(float const n) noexcept { return yh = n * h; }

        friend constexpr bool operator==(point2d l, point2d r) {
            return l.x() == r.x() and l.y() == r.y();
        }

        friend constexpr point2d operator+(point2d const l, point2d const r) {
            return {l.x() + r.x(), l.y() + r.y()};
        }
        friend constexpr point2d operator-(point2d const p) {
            return {p.xh, p.yh, -p.h};
        }
        friend constexpr point2d operator-(point2d const l, point2d const r) {
            return {l.x() - r.x(), l.y() - r.y()};
        }

        constexpr float mag2() const noexcept {
            auto const xp = x();
            auto const yp = y();
            return xp * xp + yp * yp;
        }

        /// Return theta as a proportion of a total revolution
        constexpr float theta() const noexcept {
            auto const xp = x();
            auto const yp = y();
            if (xp > 0) {
                if (yp > 0) {
                    return std::atan(yp / xp) / τ;
                } else {
                    return std::atan(yp / xp) / τ + 1.0f;
                }
            } else {
                return std::atan(yp / xp) / τ + 0.5f;
            }
        }
    };


    /// 2D matrix used for affine transformations
    class matrix final {
        std::array<float, 9> m = {1, 0, 0, 0, 1, 0, 0, 0, 1};

        constexpr matrix(
                float a,
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
        constexpr matrix() {}

        friend constexpr matrix operator*(matrix const &a, matrix const &b) {
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
        friend constexpr point2d operator*(matrix const &a, point2d const p) {
            return {a.m[0] * p.xh + a.m[1] * p.yh + a.m[2] * p.h,
                    a.m[3] * p.xh + a.m[4] * p.yh + a.m[5] * p.h,
                    a.m[6] * p.xh + a.m[7] * p.yh + a.m[8] * p.h};
        }

        friend constexpr bool
                operator==(matrix const &, matrix const &) = default;

        /// Reflect the y axis (about the x axis)
        static constexpr matrix reflect_y() {
            return matrix{1, 0, 0, 0, -1, 0, 0, 0, 1};
        }
        static constexpr matrix translate(point2d const by) {
            return matrix{1, 0, by.x(), 0, 1, by.y(), 0, 0, 1};
        }
        static constexpr matrix scale(float const factor) {
            return matrix{factor, 0, 0, 0, factor, 0, 0, 0, 1};
        }
        static constexpr matrix rotate(float const turns) {
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
        /// Reflect the y-axis, so up is down
        transform &reflect_y() {
            in = matrix::reflect_y() * in;
            out = out * matrix::reflect_y();
            return *this;
        }
        /// Scale up by the provided factor
        transform &scale(float const factor) {
            in = matrix::scale(factor) * in;
            out = out * matrix::scale(1.0f / factor);
            return *this;
        }
        /// Translate by the provided amount
        transform &translate(point2d const by) {
            in = matrix::translate(by) * in;
            out = out * matrix::translate(-by);
            return *this;
        }
        /// Rotate by the requested number of turns. One turn is τ radians
        transform &rotate(float const turns) {
            in = matrix::rotate(turns) * in;
            out = out * matrix::rotate(-turns);
            return *this;
        }

        point2d into(point2d const p) const { return in * p; }
        point2d outof(point2d const p) const { return out * p; }
    };


}
