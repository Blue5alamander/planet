#pragma once


#include <planet/affine/matrix2d.hpp>
#include <planet/affine/point3d.hpp>

#include <span>
#include <utility>


namespace planet::affine {


    /// ## 3D affine transform matrix
    class matrix3d final {
        friend class transform3d;


        /**
         * The memory layout here needs to be compatible with Vulkan's
         * requirements. This dictates the memory alignment requirement and it
         * also dictates that the layout should be in columns. That is, the
         * first 4 values are column 1 and the next 4 are column 2 etc. This is
         * a little bit awkward because it's much easier to order in terms of
         * rows
         */
        alignas(16) std::array<float, 16> m = {1, 0, 0, 0, 0, 1, 0, 0,
                                               0, 0, 1, 0, 0, 0, 0, 1};

        /// Construct the matrix from a span of 4 rows.
        constexpr matrix3d(std::array<std::array<float, 4>, 4> const &v)
        : m{v[0][0], v[1][0], v[2][0], v[3][0], v[0][1], v[1][1],
            v[2][1], v[3][1], v[0][2], v[1][2], v[2][2], v[3][2],
            v[0][3], v[1][3], v[2][3], v[3][3]} {}


      public:
        /// ### Construction
        constexpr matrix3d() {}
        constexpr matrix3d(matrix2d const &m2)
        : matrix3d{
                  {std::array{m2[{0, 0}], m2[{1, 0}], float{}, m2[{2, 0}]},
                   std::array{m2[{0, 1}], m2[{1, 1}], float{}, m2[{2, 1}]},
                   std::array{float{}, float{}, float{1}, float{}},
                   std::array{m2[{0, 2}], m2[{1, 2}], float{}, m2[{2, 2}]}}} {}

        static constexpr matrix3d
                translate(float const x, float const y, float const z) {
            return {
                    {std::array{1.0f, 0.0f, 0.0f, x},
                     std::array{0.0f, 1.0f, 0.0f, y},
                     std::array{0.0f, 0.0f, 1.0f, z},
                     std::array{0.0f, 0.0f, 0.0f, 1.0f}}};
        }
        static constexpr matrix3d translate(point3d const &p) {
            return translate(p.x(), p.y(), p.z());
        }
        static constexpr matrix3d
                scale(float const x, float const y, float const z) {
            return {
                    {std::array{x, 0.0f, 0.0f, 0.0f},
                     std::array{0.0f, y, 0.0f, 0.0f},
                     std::array{0.0f, 0.0f, z, 0.0f},
                     std::array{0.0f, 0.0f, 0.0f, 1.0f}}};
        }
        static constexpr matrix3d scale(float const s) {
            return scale(s, s, s);
        }
        static constexpr matrix3d rotate_x(float const t) {
            float const r = t * tau;
            float const c = std::cos(r);
            float const s = std::sin(r);
            return {
                    {std::array{1.0f, 0.0f, 0.0f, 0.0f},
                     std::array{0.0f, c, -s, 0.0f},
                     std::array{0.0f, s, c, 0.0f},
                     std::array{0.0f, 0.0f, 0.0f, 1.0f}}};
        }
        static constexpr matrix3d rotate_y(float const t) {
            float const r = t * tau;
            float const c = std::cos(r);
            float const s = std::sin(r);
            return {
                    {std::array{c, 0.0f, s, 0.0f},
                     std::array{0.0f, 1.0f, 0.0f, 0.0f},
                     std::array{-s, 0.0f, c, 0.0f},
                     std::array{0.0f, 0.0f, 0.0f, 1.0f}}};
        }
        static constexpr matrix3d rotate_z(float const t) {
            float const r = t * tau;
            float const c = std::cos(r);
            float const s = std::sin(r);
            return {
                    {std::array{c, -s, 0.0f, 0.0f},
                     std::array{s, c, 0.0f, 0.0f},
                     std::array{0.0f, 0.0f, 1.0f, 0.0f},
                     std::array{0.0f, 0.0f, 0.0f, 1.0f}}};
        }


        friend constexpr bool
                operator==(matrix3d const &, matrix3d const &) = default;


        constexpr float operator[](
                std::pair<std::size_t, std::size_t> const s) const noexcept {
            return m[(s.first bitand 3) * 4 + (s.second bitand 3)];
        }


        /// ### Multiplication
        friend constexpr matrix3d
                operator*(matrix3d const &a, matrix3d const &b) {
            float const a0 = a.m[0], a1 = a.m[1], a2 = a.m[2], a3 = a.m[3],
                        a4 = a.m[4], a5 = a.m[5], a6 = a.m[6], a7 = a.m[7],
                        a8 = a.m[8], a9 = a.m[9], a10 = a.m[10], a11 = a.m[11],
                        a12 = a.m[12], a13 = a.m[13], a14 = a.m[14],
                        a15 = a.m[15];

            float const b0 = b.m[0], b1 = b.m[1], b2 = b.m[2], b3 = b.m[3],
                        b4 = b.m[4], b5 = b.m[5], b6 = b.m[6], b7 = b.m[7],
                        b8 = b.m[8], b9 = b.m[9], b10 = b.m[10], b11 = b.m[11],
                        b12 = b.m[12], b13 = b.m[13], b14 = b.m[14],
                        b15 = b.m[15];

            return {
                    {std::array{
                             a0 * b0 + a4 * b1 + a8 * b2 + a12 * b3,
                             a0 * b4 + a4 * b5 + a8 * b6 + a12 * b7,
                             a0 * b8 + a4 * b9 + a8 * b10 + a12 * b11,
                             a0 * b12 + a4 * b13 + a8 * b14 + a12 * b15,
                     },
                     std::array{
                             a1 * b0 + a5 * b1 + a9 * b2 + a13 * b3,
                             a1 * b4 + a5 * b5 + a9 * b6 + a13 * b7,
                             a1 * b8 + a5 * b9 + a9 * b10 + a13 * b11,
                             a1 * b12 + a5 * b13 + a9 * b14 + a13 * b15,
                     },
                     std::array{
                             a2 * b0 + a6 * b1 + a10 * b2 + a14 * b3,
                             a2 * b4 + a6 * b5 + a10 * b6 + a14 * b7,
                             a2 * b8 + a6 * b9 + a10 * b10 + a14 * b11,
                             a2 * b12 + a6 * b13 + a10 * b14 + a14 * b15,
                     },
                     std::array{
                             a3 * b0 + a7 * b1 + a11 * b2 + a15 * b3,
                             a3 * b4 + a7 * b5 + a11 * b6 + a15 * b7,
                             a3 * b8 + a7 * b9 + a11 * b10 + a15 * b11,
                             a3 * b12 + a7 * b13 + a11 * b14 + a15 * b15,
                     }}};
        }

        friend constexpr point3d operator*(matrix3d const &a, point3d const &p) {
            float const a0 = a.m[0], a1 = a.m[1], a2 = a.m[2], a3 = a.m[3],
                        a4 = a.m[4], a5 = a.m[5], a6 = a.m[6], a7 = a.m[7],
                        a8 = a.m[8], a9 = a.m[9], a10 = a.m[10], a11 = a.m[11],
                        a12 = a.m[12], a13 = a.m[13], a14 = a.m[14],
                        a15 = a.m[15];
            return {p.xh * a0 + p.yh * a4 + p.zh * a8 + p.h * a12,
                    p.xh * a1 + p.yh * a5 + p.zh * a9 + p.h * a13,
                    p.xh * a2 + p.yh * a6 + p.zh * a10 + p.h * a14,
                    p.xh * a3 + p.yh * a7 + p.zh * a11 + p.h * a15};
        }
    };


}
