#pragma once


#include <planet/affine/point2d.hpp>


namespace planet::affine {


    /// ## 3D point for affine transforms
    struct point3d final {
        float xh = {}, yh = {}, zh = {}, h = {1.0f};


        /// ### Construction
        constexpr point3d() {}
        constexpr point3d(float const x, float const y, float const z)
        : xh{x}, yh{y}, zh{z} {}
        constexpr point3d(
                float const x, float const y, float const z, float const h)
        : xh{x}, yh{y}, zh{z}, h{h} {}
        constexpr point3d(point2d const &p, float const z = {})
        : xh{p.xh}, yh{p.yh}, zh{z * p.h}, h{p.h} {}

        static point3d unit_vector(point3d const &p) noexcept {
            return p.as_unit_vector();
        }


        /// ### Queries
        constexpr float x() const noexcept { return xh / h; }
        constexpr float x(float const n) noexcept { return xh = n * h; }
        constexpr float y() const noexcept { return yh / h; }
        constexpr float y(float const n) noexcept { return yh = n * h; }
        constexpr float z() const noexcept { return zh / h; }
        constexpr float z(float const n) noexcept { return zh = n * h; }

        constexpr point2d xy() const noexcept { return {x(), y()}; }

        /// #### Square of the distance from origin
        constexpr float mag2() const noexcept {
            return x() * x() + y() * y() + z() * z();
        }

        /// #### Dot product
        constexpr float dot(point3d const &p) const noexcept {
            return x() * p.x() + y() * p.y() + z() * p.z();
        }


        /// ### Operators

        /// #### Maths operations
        friend constexpr point3d operator-(point3d const &p) {
            return {p.xh, p.yh, p.zh, -p.h};
        }
        friend constexpr point3d operator-(point3d const &l, point3d const &r) {
            return {l.x() - r.x(), l.y() - r.y(), l.z() - r.z()};
        }

        point3d &operator+=(point3d const &r) { return *this = *this + r; }
        friend constexpr point3d operator+(point3d const &l, point3d const &r) {
            return {l.x() + r.x(), l.y() + r.y(), l.z() + r.z()};
            // return {l.xh * r.h + r.xh * l.h, l.yh * r.h + r.yh * l.h,
            //         l.zh * r.h + r.zh * l.h, l.h * r.h};
        }

        point3d &operator/=(float const s) {
            xh /= s;
            yh /= s;
            zh /= s;
            return *this;
        }
        friend point3d operator/(point3d const &p, float const s) {
            return {p.xh / s, p.yh / s, p.zh / s, p.h};
        }

        friend constexpr point3d operator*(point3d const &l, float const r) {
            return {l.xh * r, l.yh * r, l.zh * r, l.h};
        }

        /// #### Unit vector
        /// TODO Make constexpr when g++ catches up
        point3d as_unit_vector() const noexcept {
            return *this / std::sqrt(mag2());
        }
    };


}
