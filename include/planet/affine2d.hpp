#pragma once


#include <array>


namespace planet {


    /// 2D point suitable for affine transformations
    class point2d final {
        float xh = {}, yh = {}, h = {1.0f};

      public:
        point2d(float x, float y) : xh{x}, yh{y} {}

        float x() const noexcept { return xh / h; }
        float y() const noexcept { return yh / h; }
    };


    /// 2D matrix used for affine transformations
    class transform final {
        std::array<float, 9> matrix = {};

      public:
    };


}
