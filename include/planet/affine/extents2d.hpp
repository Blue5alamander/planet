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

        constexpr std::size_t zwidth() const noexcept {
            return std::size_t(width);
        }
        constexpr std::size_t zheight() const noexcept {
            return std::size_t(height);
        }

        /// An extents is smaller only if both axes are
        constexpr bool fits_within(extents2d const r) const noexcept {
            return width <= r.width and height <= r.height;
        }

        friend bool operator==(extents2d const &, extents2d const &) = default;

        /// Scaling with a scalar
        friend constexpr extents2d
                operator*(extents2d const e, float const f) noexcept {
            return {e.width * f, e.height * f};
        }
        /// Addition of points
        friend constexpr point2d
                operator+(point2d const p, extents2d const e) noexcept {
            return {p.x() + e.width, p.y() + e.height};
        }
    };


}
