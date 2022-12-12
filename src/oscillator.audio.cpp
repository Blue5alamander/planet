#include <planet/audio/clocks.hpp>
#include <planet/audio/oscillator.hpp>

#include <complex>
#if __has_include(<numbers>)
#include <numbers>
namespace {
    constexpr auto tau = std::numbers::pi_v<float> * 2.0f;
}
#else
namespace {
    constexpr auto tau = 6.283185307179586f;
}
#endif


felspar::coro::generator<std::span<float>> silence() {
    std::array<float, planet::audio::default_buffer_samples> buffer{};
    while (true) { co_yield buffer; }
}


felspar::coro::generator<std::span<float>>
        planet::audio::oscillator(float const turns) {
    std::array<float, planet::audio::default_buffer_samples> buffer;
    std::complex const rotate{std::cos(turns * tau), std::sin(turns * tau)};
    std::complex phase{1.0f, 0.0f};
    while (true) {
        for (auto &s : buffer) {
            s = phase.imag();
            phase *= rotate;
        }
        co_yield buffer;
    }
}
