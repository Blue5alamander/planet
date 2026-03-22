#include <planet/audio/gain.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("audio", [](auto check) {
        auto const gain = planet::audio::dB_gain{-3};
        planet::audio::linear_gain const linear{gain};
        // Expect ~0.707f
        check(linear.load()) > 0.707f;
        check(linear.load()) < 0.708f;

        // Round-trip: linear -> dB should recover the original value
        planet::audio::dB_gain const back{linear};
        check(back.dB) > -3.01f;
        check(back.dB) < -2.99f;

        // Zero/negative linear gain -> silent floor
        planet::audio::linear_gain const zero{0.0f};
        check(planet::audio::dB_gain{zero}.dB) == -128.0f;
    });


}
