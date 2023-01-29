#pragma once


#include <array>
#include <span>
#include <utility>


namespace planet::affine {


    /// ## 3D affine transform matrix
    class matrix3d final {
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
        matrix3d(std::array<std::array<float, 4>, 4> const &v)
        : m{v[0][0], v[1][0], v[2][0], v[3][0],

            v[0][1], v[1][1], v[2][1], v[3][1],

            v[0][2], v[1][2], v[2][2], v[3][2],

            v[0][3], v[1][3], v[2][3], v[3][3]} {}

      public:
        matrix3d() {}

        float operator[](std::pair<std::size_t, std::size_t> const s) const {
            return m[s.first * 4 + s.second];
        }

        static matrix3d translate(float const x, float const y, float const z) {
            return {
                    {std::array{1.0f, 0.0f, 0.0f, x},
                     std::array{0.0f, 1.0f, 0.0f, y},
                     std::array{0.0f, 0.0f, 1.0f, z},
                     std::array{0.0f, 0.0f, 0.0f, 1.0f}}};
        }
        static matrix3d scale_xy(float const x, float const y) {
            return {
                    {std::array{x, 0.0f, 0.0f, 0.0f},
                     std::array{0.0f, y, 0.0f, 0.0f},
                     std::array{0.0f, 0.0f, 1.0f, 0.0f},
                     std::array{0.0f, 0.0f, 0.0f, 1.0f}}};
        }
    };


}
