#pragma once


#include <planet/affine/point2d.hpp>


namespace planet::affine {


    /// ## 3D point for affine transforms
    struct point3d final {
        float xh = {}, yh = {}, zh = {}, h = {1.0f};

        constexpr point3d() {}
        constexpr point3d(float const x, float const y) : xh{x}, yh{y} {}
        constexpr point3d(point2d const &p) : xh{p.xh}, yh{p.yh}, h{p.h} {}


        constexpr float x() const noexcept { return xh / h; }
        constexpr float x(float const n) noexcept { return xh = n * h; }
        constexpr float y() const noexcept { return yh / h; }
        constexpr float y(float const n) noexcept { return yh = n * h; }
        constexpr float z() const noexcept { return zh / h; }
        constexpr float z(float const n) noexcept { return zh = n * h; }
    };


}
