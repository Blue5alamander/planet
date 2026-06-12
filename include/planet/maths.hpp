#pragma once


#include <planet/numbers.hpp>

#include <cstdint>


namespace planet {


    /// ## Integer-turn trigonometry
    /**
     * `sin`/`cos` take a `std::uint32_t` representing a fraction of a full turn
     * (`1 / 2^32`). The top 12 bits select an entry in a 4096-point sine lookup
     * table; the low 20 bits drive linear interpolation between adjacent
     * entries. `cos(t)` is computed as `sin(t + 2^30)` (one quarter turn), so
     * the two functions share a single LUT and unsigned wrap-around handles the
     * quadrant shift for free.
     */
    [[nodiscard]] float sin(std::uint32_t) noexcept;
    [[nodiscard]] float cos(std::uint32_t) noexcept;


    /// ### Turn fraction to integer turn
    /**
     * Convert a turn fraction (`1.0f` is one full turn) into the integer-turn
     * representation consumed by `sin`/`cos`. Turn fractions outside `[0, 1)`
     * wrap around via unsigned modular arithmetic.
     */
    [[nodiscard]] inline std::uint32_t turns(double const t) noexcept {
        return static_cast<std::uint32_t>(
                static_cast<std::int64_t>(t * 0x1p32));
    }
    [[nodiscard]] inline std::uint32_t turns(float const t) noexcept {
        return turns(static_cast<double>(t));
    }


    /// ### Turn-fraction overloads
    /**
     * Convenience overloads taking a turn fraction (`1.0f` is one full turn).
     * They convert via `turns` and look the result up in the same LUT, so they
     * share the integer functions' accuracy (worst-case error ≈ 6e-7).
     */
    [[nodiscard]] inline float sin(double const turn) noexcept {
        return sin(turns(turn));
    }
    [[nodiscard]] inline float cos(double const turn) noexcept {
        return cos(turns(turn));
    }
    [[nodiscard]] inline float sin(float const turn) noexcept {
        return sin(turns(turn));
    }
    [[nodiscard]] inline float cos(float const turn) noexcept {
        return cos(turns(turn));
    }


}
