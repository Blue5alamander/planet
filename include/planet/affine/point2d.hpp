#pragma once


#include <planet/numbers.hpp>

#include <cmath>
#include <complex>


namespace planet::affine {


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
        friend constexpr point2d operator*(point2d const p, float const s) {
            return {p.xh, p.yh, p.h / s};
        }
        friend constexpr point2d operator/(point2d const p, float const s) {
            return {p.xh, p.yh, p.h * s};
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
            if (xp == 0) {
                if (yp > 0) {
                    return 0.25f;
                } else if (yp < 0) {
                    return 0.75f;
                } else {
                    return 0.0f;
                }
            } else if (xp > 0) {
                if (yp > 0) {
                    return std::atan(yp / xp) / τ;
                } else if (yp == 0) {
                    return 0.0f;
                } else {
                    return std::atan(yp / xp) / τ + 1.0f;
                }
            } else {
                return std::atan(yp / xp) / τ + 0.5f;
            }
        }
        /// Rotate a point about the origin
        point2d rotate(float const theta) {
            std::complex const rotate{std::cos(theta * τ), std::sin(theta * τ)};
            std::complex const value{xh, yh};
            auto const result = value * rotate;
            return {result.real(), result.imag(), h};
        }
    };


}
