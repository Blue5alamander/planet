#include <planet/maths.hpp>
#include <planet/numbers.hpp>
#include <planet/functional.hpp>

#include <array>
#include <cmath>


/// ## `planet::sin` / `planet::cos`


namespace {
    std::size_t constexpr trig_index_bits = 12;
    std::size_t constexpr trig_entries = std::size_t{1} << trig_index_bits;
    std::uint32_t constexpr trig_index_shift = 32u - trig_index_bits;
    std::uint32_t constexpr trig_frac_mask =
            (std::uint32_t{1} << trig_index_shift) - 1u;
    float constexpr trig_frac_scale =
            1.0f / float(std::uint32_t{1} << trig_index_shift);


    /**
     * One extra entry so the interpolation neighbour never needs a modulo.
     * `g_sin_lut[trig_entries] == g_sin_lut[0]` by construction.
     */
    std::array<float, trig_entries + 1> const g_sin_lut = [] {
        std::array<float, trig_entries + 1> table{};
        planet::by_index(trig_entries + 1, [&](std::size_t const i) {
            table[i] = std::sin(planet::tau * float(i) / float(trig_entries));
        });
        return table;
    }();
}


float planet::sin(std::uint32_t const turn) noexcept {
    auto const index = turn >> trig_index_shift;
    auto const frac = turn bitand trig_frac_mask;
    auto const a = g_sin_lut[index];
    auto const b = g_sin_lut[index + 1];
    return a + (b - a) * float(frac) * trig_frac_scale;
}


float planet::cos(std::uint32_t const turn) noexcept {
    return sin(turn + (std::uint32_t{1} << 30));
}
