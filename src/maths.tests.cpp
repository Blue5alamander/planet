#include <planet/maths.hpp>

#include <felspar/test.hpp>

#include <cmath>
#include <numbers>


namespace {


    std::uint32_t constexpr quarter = std::uint32_t{1} << 30;
    double constexpr tau = 2.0 * std::numbers::pi_v<double>;
    double constexpr turn_scale = tau / 4294967296.0;
    float constexpr error = 1e-6f;


    float reference_sin(std::uint32_t const t) noexcept {
        return float(std::sin(turn_scale * double(t)));
    }


    auto const trig = felspar::testsuite(
            "trig",
            [](auto check) {
                check(planet::sin(0u)) == 0.0f;
                check(std::abs(planet::sin(quarter) - 1.0f)) < error;
                check(std::abs(planet::sin(quarter * 2u))) < error;
                check(std::abs(planet::sin(quarter * 3u) + 1.0f)) < error;

                check(std::abs(planet::cos(0u) - 1.0f)) < error;
                check(std::abs(planet::cos(quarter))) < error;
                check(std::abs(planet::cos(quarter * 2u) + 1.0f)) < error;
                check(std::abs(planet::cos(quarter * 3u))) < error;
            },
            [](auto check) {
                /**
                 * `cos` is defined as `sin(t + 1/4 turn)`; the identity must
                 * hold exactly, not just within tolerance.
                 */
                for (std::uint32_t i = 0; i < 4096; ++i) {
                    auto const t = i * (std::uint32_t{1} << 20);
                    check(planet::cos(t)) == planet::sin(t + quarter);
                }
            },
            [](auto check) {
                /**
                 * Sweep ~65k points across the full turn and compare against
                 * `std::sin`/`std::cos`. Worst-case linear-interp error is
                 * ~6e-7; 1e-5 leaves ample margin for float arithmetic.
                 */
                for (std::uint32_t i = 0; i < (std::uint32_t{1} << 16); ++i) {
                    auto const t = i << 16;
                    check(std::abs(planet::sin(t) - reference_sin(t))) < error;
                    check(std::abs(planet::cos(t) - reference_sin(t + quarter)))
                            < error;
                }
            },
            [](auto check) {
                /// `sin² + cos² ≈ 1` everywhere on the unit circle.
                for (std::uint32_t i = 0; i < 4096; ++i) {
                    auto const t = i * (std::uint32_t{1} << 20);
                    auto const s = planet::sin(t);
                    auto const c = planet::cos(t);
                    check(std::abs(s * s + c * c - 1.0f)) < error;
                }
            },
            [](auto check) {
                /// `turns` maps a turn fraction onto the integer-turn phase.
                check(planet::turns(0.0f)) == 0u;
                check(planet::turns(0.25f)) == quarter;
                check(planet::turns(0.5f)) == quarter * 2u;
                /// Fractions outside `[0, 1)` wrap via unsigned modulo.
                check(planet::turns(1.0f)) == 0u;
                check(planet::turns(-0.25f)) == quarter * 3u;
            },
            [](auto check) {
                /**
                 * The turn-fraction overloads share the LUT's accuracy and
                 * agree with the integer functions for matching phases.
                 */
                check(std::abs(planet::cos(0.0f) - 1.0f)) < error;
                check(std::abs(planet::sin(0.25f) - 1.0f)) < error;
                check(planet::sin(0.25f)) == planet::sin(quarter);
                check(planet::cos(0.125f)) == planet::cos(quarter / 2u);
            });


}
