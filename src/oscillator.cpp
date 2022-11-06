#include <planet/audio/oscillator.hpp>

#include <complex>
#include <numbers>


namespace {
    constexpr auto tau = std::numbers::pi_v<float> * 2.0f;
}


felspar::coro::generator<std::span<float>> silence() {
    std::array<float, 2 << 10> buffer{};
    while (true) { co_yield buffer; }
}


felspar::coro::generator<std::span<float>>
        planet::audio::oscillator(float const turns) {
    std::array<float, 2 << 10> buffer;
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
