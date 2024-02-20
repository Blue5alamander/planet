#pragma once


#include <planet/affine/point2d.hpp>

#include <array>
#include <span>


namespace planet::affine {


    /// ## 2D affine transform matrix
    class matrix2d final {
        std::array<float, 9> m = {1, 0, 0, 0, 1, 0, 0, 0, 1};

        constexpr matrix2d(
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
        constexpr matrix2d() {}

        friend constexpr matrix2d
                operator*(matrix2d const &a, matrix2d const &b) {
            return matrix2d{
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
        friend constexpr point2d operator*(matrix2d const &a, point2d const p) {
            return {a.m[0] * p.xh + a.m[1] * p.yh + a.m[2] * p.h,
                    a.m[3] * p.xh + a.m[4] * p.yh + a.m[5] * p.h,
                    a.m[6] * p.xh + a.m[7] * p.yh + a.m[8] * p.h};
        }

        friend constexpr bool
                operator==(matrix2d const &, matrix2d const &) = default;


        /// ### Named constructors
        static constexpr matrix2d reflect_y() {
            return {1, 0, 0, 0, -1, 0, 0, 0, 1};
        }
        static constexpr matrix2d translate(point2d const by) {
            return {1, 0, by.x(), 0, 1, by.y(), 0, 0, 1};
        }
        static constexpr matrix2d scale(float const factor) {
            return {factor, 0, 0, 0, factor, 0, 0, 0, 1};
        }
        static constexpr matrix2d scale(float const x, float const y) {
            return {x, 0, 0, 0, y, 0, 0, 0, 1};
        }
        static constexpr matrix2d rotate(float const turns) {
            float const r = turns * tau;
            return {std::cos(r),
                    -std::sin(r),
                    0,

                    std::sin(r),
                    std::cos(r),
                    0,

                    0,
                    0,
                    1};
        }


        /// ### Access into the matrix
        constexpr float operator[](
                std::pair<std::size_t, std::size_t> const p) const noexcept {
            return m[(p.first % 3) + (p.second % 3) * 3];
        }

        constexpr std::span<float const, 9> cmemory() const noexcept {
            return m;
        }
        constexpr std::span<float, 9> memory() noexcept { return m; }
    };


}
