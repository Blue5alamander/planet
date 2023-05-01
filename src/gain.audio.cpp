#include <planet/audio/gain.hpp>

#include <cmath>


/// ## `planet::audio::linear_gain`


planet::audio::linear_gain::linear_gain(float const g) : multiplier{g} {}


/// ## `planet::audio::atomic_linear_gain`


void planet::audio::atomic_linear_gain::set(linear_gain const g) {
    multiplier = g.multiplier;
}


/// ## `planet::audio::dB_gain`


planet::audio::dB_gain::dB_gain(float const g) : dB{g} {}


planet::audio::dB_gain::operator linear_gain() const noexcept {
    if (dB < -200.0f) {
        return linear_gain{};
    } else {
        return linear_gain{std::pow(10.0f, dB / 20.0f)};
    }
}
