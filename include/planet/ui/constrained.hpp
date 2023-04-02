#pragma once


#include <planet/affine/extents2d.hpp>

#include <concepts>
#include <limits>


namespace planet {


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

        template<std::equality_comparable_with<value_type> R>
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
    class constrained2d {
        constrained1d<T> width, height;

      public:
        using value_type = T;

        constrained2d() noexcept {}
        constrained2d(value_type w, value_type h) noexcept
        : width{w}, height{h} {}
        explicit constrained2d(affine::extents2d ex) noexcept
        : width{ex.width, ex.width, ex.width},
          height{ex.height, ex.width, ex.height} {}

        affine::extents2d extents() const noexcept {
            return {width.value(), height.value()};
        }

        friend bool operator==(constrained2d const &l, constrained2d const &r) {
            return l.width == r.width and l.height == r.height;
        }
    };


}
