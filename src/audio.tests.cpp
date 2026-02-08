#include <planet/audio/gain.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("audio", [](auto check) {
        auto const gain = planet::audio::dB_gain{-3};
        planet::audio::linear_gain const linear{gain};
        check(linear.load()) > 0.49f;
        check(linear.load()) < 0.51f;
    });


}
