#pragma once


#include <planet/affine/extents2d.hpp>

#include <limits>
#include <optional>


namespace planet::ui {


    /// ## Constrained scalar values
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


        /// ### Construction
        constexpr constrained1d() noexcept {}
        explicit constexpr constrained1d(value_type t, value_type min = {})
        : m_value{t}, m_desired{t}, m_min{min} {}
        constexpr constrained1d(
                value_type t, value_type min, value_type max) noexcept
        : m_value{t}, m_desired{t}, m_min{min}, m_max{max} {}


        /// ### Access to held values
        value_type value() const noexcept { return m_value; }
        value_type min() const noexcept { return m_min; }
        value_type max() const noexcept { return m_max; }
        /// #### The normalised value to a scale of 0 to 1
        value_type normalised_value() const noexcept {
            if (m_max != m_min) {
                return (m_value - m_min) / (m_max - m_min);
            } else {
                return {};
            }
        }
        /// #### The current value remapped to another constrained
        value_type remapped_to(constrained1d const &c) const noexcept {
            return c.m_min + (normalised_value() * (c.m_max - c.m_min));
        }


        /// ### Change held values
        value_type desire(value_type const m) {
            m_desired = m;
            return constrain();
        }
        value_type min(value_type const m) {
            m_min = m;
            return constrain();
        }
        value_type max(value_type const m) {
            m_max = m;
            return constrain();
        }


        /// ### Comparisons
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


    /// ## Maximum constraints
    template<typename T>
    constrained1d<T>
            maximum_of(constrained1d<T> const &l, constrained1d<T> const &r) {
        return {std::max(l.value(), r.value()), std::max(l.min(), r.min()),
                std::max(l.max(), r.max())};
    }


    /// ## Constrained area
    template<typename T>
    struct constrained2d {
        using value_type = T;
        using axis_constrained_type = constrained1d<value_type>;
        axis_constrained_type width, height;


        /// ### Construction
        constexpr constrained2d() noexcept {}
        constexpr constrained2d(value_type w, value_type h) noexcept
        : width{w}, height{h} {}
        constexpr constrained2d(
                axis_constrained_type const &w, axis_constrained_type const &h)
        : width{w}, height{h} {}
        constexpr constrained2d(
                affine::extents2d const ex,
                affine::extents2d const min,
                affine::extents2d const max)
        : width(ex.width, min.width, max.width),
          height{ex.height, min.height, max.height} {}
        /// #### Force a fixed size with no latitude for change
        explicit constexpr constrained2d(affine::extents2d const ex) noexcept
        : width{ex.width, ex.width, ex.width},
          height{ex.height, ex.height, ex.height} {}


        /// ### Conversions
        affine::extents2d extents() const noexcept {
            return {width.value(), height.value()};
        }
        affine::point2d position() const noexcept {
            return {width.value(), height.value()};
        }

        affine::extents2d min_extents() const noexcept {
            return {width.min(), height.min()};
        }
        affine::extents2d max_extents() const noexcept {
            return {width.max(), height.max()};
        }
        affine::point2d min_position() const noexcept {
            return {width.min(), height.min()};
        }
        affine::point2d max_position() const noexcept {
            return {width.max(), height.max()};
        }


        /// ### Change
        affine::point2d desire(affine::point2d const &p) noexcept {
            return {width.desire(p.x()), height.desire(p.y())};
        }
        affine::extents2d desire(affine::extents2d const &e) noexcept {
            return {width.desire(e.width), height.desire(e.height)};
        }
        void max(affine::extents2d const &e) noexcept {
            width.max(e.width);
            height.max(e.height);
        }


        /// ### Queries
        bool is_at_least_as_constrained_as(constrained2d const &o) const {
            return width.is_at_least_as_constrained_as(o.width)
                    and height.is_at_least_as_constrained_as(o.height);
        }

        friend bool operator==(constrained2d const &l, constrained2d const &r) {
            return l.width == r.width and l.height == r.height;
        }
    };


}
