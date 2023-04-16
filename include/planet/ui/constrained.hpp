#pragma once


#include <planet/affine/extents2d.hpp>

// #include <concepts>
#include <limits>
#include <optional>


namespace planet::ui {


    /// ## Constrained values
    template<typename T>
    class constrained1d {
        T m_value{}, m_desired{}, m_min{},
                m_max{std::numeric_limits<value_type>::max()};

        auto constrain() {
            m_value = m_desired;
            if (m_value < m_min) {
                m_value = m_min;
            } else if (m_value > m_max) {
                m_value = m_max;
            }
            return m_value;
        }

      public:
        using value_type = T;

        constrained1d() noexcept {}
        explicit constrained1d(value_type t, value_type min = {})
        : m_value{t}, m_desired{t}, m_min{min} {}
        constrained1d(value_type t, value_type min, value_type max) noexcept
        : m_value{t}, m_desired{t}, m_min{min}, m_max{max} {}

        value_type value() const noexcept { return m_value; }
        value_type min() const noexcept { return m_min; }
        value_type max() const noexcept { return m_max; }

        value_type min(value_type const m) {
            m_min = m;
            return constrain();
        }
        value_type max(value_type const m) {
            m_max = m;
            return constrain();
        }

        bool is_at_least_as_constrained_as(constrained1d const &o) const {
            return m_min >= o.m_min and m_max <= o.m_max;
        }

        // TODO std::equality_comparable_with<value_type>
        // When android NDK gets the concepts header
        template<typename R>
        friend bool
                operator==(constrained1d const &l, constrained1d<R> const &r) {
            return l.m_value == r.value();
        }
        friend bool operator==(constrained1d const &l, value_type const r) {
            return l.m_value == r;
        }
    };


    /// ## Constrained area
    template<typename T>
    struct constrained2d {
        using value_type = T;
        constrained1d<value_type> width, height;

        constrained2d() noexcept {}
        constrained2d(value_type w, value_type h) noexcept
        : width{w}, height{h} {}
        explicit constrained2d(affine::extents2d const ex) noexcept
        : width{ex.width, {}, ex.width}, height{ex.height, {}, ex.height} {}
        constrained2d(
                affine::extents2d const ex,
                affine::extents2d const min,
                affine::extents2d const max)
        : width(ex.width, min.width, max.width),
          height{ex.height, min.height, max.height} {}

        affine::extents2d extents() const noexcept {
            return {width.value(), height.value()};
        }
        affine::extents2d min() const noexcept {
            return {width.min(), height.min()};
        }
        affine::extents2d max() const noexcept {
            return {width.max(), height.max()};
        }

        bool is_at_least_as_constrained_as(constrained2d const &o) const {
            return width.is_at_least_as_constrained_as(o.width)
                    and height.is_at_least_as_constrained_as(o.height);
        }

        friend bool operator==(constrained2d const &l, constrained2d const &r) {
            return l.width == r.width and l.height == r.height;
        }
    };


}
