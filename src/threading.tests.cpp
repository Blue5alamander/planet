#include <planet/threading.hpp>

#include <felspar/test.hpp>

#include <limits>


namespace {


    auto const suite = felspar::testsuite("threading");


    auto const denormals =
            suite.test("flush_denormals_to_zero", [](auto check) {
                planet::threading::flush_denormals_to_zero();
                /**
                 * Halving the smallest normal `float` yields a denormal. Once
                 * the calling thread flushes denormals to zero the hardware
                 * rounds that result to zero instead. `volatile` keeps the
                 * arithmetic at run time so the live floating point mode
                 * applies rather than a compile-time constant.
                 */
                volatile float value = std::numeric_limits<float>::min();
                value = value * 0.5f;
                float const result = value;
                check(result) == 0.0f;
            });


}
