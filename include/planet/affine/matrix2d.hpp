#pragma once


#include <planet/affine/point2d.hpp>

#include <array>


namespace planet::affine {


    /// 2D matrix used for affine transformations
    class matrix final {
        std::array<float, 9> m = {1, 0, 0, 0, 1, 0, 0, 0, 1};

        constexpr matrix(
                float const a,
                float const b,
                float const c,
                float const d,
                float const e,
                float const f,
                float const g,
                float const h,
                float const i)
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


}
