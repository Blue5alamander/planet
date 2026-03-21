#pragma once


#include <chrono>


namespace planet::audio {


    using namespace std::literals;


    /// ## Standard audio output sample rate
    using sample_clock =
            std::chrono::duration<std::int64_t, std::ratio<1, 48'000>>;

    std::size_t constexpr samples_per_second = sample_clock::period::den;
    std::size_t constexpr nyquist_limit = samples_per_second / 2;


    /// ## The amount of time for a standard buffer slice
    sample_clock constexpr default_buffer_duration{512};
    /**
     * The audio system in SDL2 wants to use 512 as a fast clock time, so we'll
     * align to that. We could go even smaller for even lower latency on systems
     * that are fast enough for that.
     */
    std::size_t constexpr default_buffer_samples =
            default_buffer_duration.count();


}
