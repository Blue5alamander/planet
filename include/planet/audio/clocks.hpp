#pragma once


#include <chrono>


namespace planet::audio {


    using namespace std::literals;


    /// ## Standard audio output sample rate
    using sample_clock =
            std::chrono::duration<std::int64_t, std::ratio<1, 48'000>>;

    std::size_t constexpr samples_per_second = sample_clock::period::den;


    /// ## The amount of time for a standard buffer slice
    sample_clock constexpr default_buffer_duration = 20ms;
    std::size_t constexpr default_buffer_samples =
            default_buffer_duration.count();


}
