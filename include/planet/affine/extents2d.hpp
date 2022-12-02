#pragma once


#include <planet/affine/point2d.hpp>


namespace planet::affine {


    /// Extents of a rectangular area
    struct extents2d {
        float width, height;

        constexpr extents2d(float const w, float const h)
        : width{w}, height{h} {}
        explicit constexpr extents2d(point2d const p)
        : width{p.x()}, height{p.y()} {}

        std::size_t zwidth() const { return std::size_t(width); }
        std::size_t zheight() const { return std::size_t(height); }
    };


}
