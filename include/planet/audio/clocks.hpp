#pragma once


#include <chrono>


namespace planet::audio {


    using namespace std::literals;


    /// Standard audio output sample rate
    using sample_clock =
            std::chrono::duration<std::int64_t, std::ratio<1, 48'000>>;


    /// The amount of time for a buffer slice
    constexpr sample_clock default_buffer_duration = 20ms;
    constexpr std::size_t default_buffer_samples =
            default_buffer_duration.count();


}
