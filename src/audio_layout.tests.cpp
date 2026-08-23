#include <planet/audio/layout.hpp>
#include <planet/audio/stereo.hpp>
#include <planet/functional.hpp>
#include <planet/log.hpp>

#include <felspar/test.hpp>

#include <array>


namespace {


    auto const suite = felspar::testsuite("audio_layout", []() {
        planet::log::active = planet::log::level::error;
    });


    std::size_t constexpr channels = planet::audio::stereo_buffer::channels;
    std::size_t constexpr frames = 4;


    /// `interleave` lays planar channels out as `{l, r, l, r, ...}`.
    auto const interleaved = suite.test("interleave", [](auto check) {
        std::array<float, frames> const left{0.0f, 1.0f, 2.0f, 3.0f};
        std::array<float, frames> const right{10.0f, 11.0f, 12.0f, 13.0f};
        std::array<float, frames * channels> out{};

        planet::audio::interleave(
                left.data(), right.data(), out.data(), frames);

        planet::by_index(frames, [&](std::size_t const f) {
            check(out[f * channels + 0]) == left[f];
            check(out[f * channels + 1]) == right[f];
        });
    });


    /// `deinterleave` splits `{l, r, ...}` back into planar channels.
    auto const deinterleaved = suite.test("deinterleave", [](auto check) {
        std::array<float, frames * channels> const in{0.0f, 0.5f, 1.0f, 1.5f,
                                                      2.0f, 2.5f, 3.0f, 3.5f};
        std::array<float, frames> left{}, right{};

        planet::audio::deinterleave(
                in.data(), left.data(), right.data(), frames);

        planet::by_index(frames, [&](std::size_t const f) {
            check(left[f]) == in[f * channels + 0];
            check(right[f]) == in[f * channels + 1];
        });
    });


    /// `deinterleave` then `interleave` reproduces the original block.
    auto const round_trip = suite.test("round_trip", [](auto check) {
        std::array<float, frames * channels> const original{
                0.1f, -0.2f, 0.3f, -0.4f, 0.5f, -0.6f, 0.7f, -0.8f};
        std::array<float, frames> left{}, right{};
        std::array<float, frames * channels> restored{};

        planet::audio::deinterleave(
                original.data(), left.data(), right.data(), frames);
        planet::audio::interleave(
                left.data(), right.data(), restored.data(), frames);

        check(restored == original) == true;
    });


}
