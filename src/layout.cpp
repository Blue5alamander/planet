#include <planet/audio/layout.hpp>
#include <planet/audio/stereo.hpp>
#include <planet/functional.hpp>


void planet::audio::deinterleave(
        float const *const interleaved,
        float *const left,
        float *const right,
        std::size_t const frames) noexcept {
    planet::by_index(frames, [&](std::size_t const f) {
        left[f] = interleaved[f * stereo_buffer::channels + 0];
        right[f] = interleaved[f * stereo_buffer::channels + 1];
    });
}


void planet::audio::interleave(
        float const *const left,
        float const *const right,
        float *const interleaved,
        std::size_t const frames) noexcept {
    planet::by_index(frames, [&](std::size_t const f) {
        interleaved[f * stereo_buffer::channels + 0] = left[f];
        interleaved[f * stereo_buffer::channels + 1] = right[f];
    });
}
