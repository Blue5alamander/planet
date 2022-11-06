#pragma once


#include <chrono>


namespace planet::audio {


    /// Audio duration in samples compatible with std::chrono durations
    using duration = std::chrono::duration<std::int64_t, std::ratio<1, 48'000>>;


    /// Audio buffer
    /**
     * * 48KHz
     * * Stereo
     * * Interleaved
     */
    class buffer {};


}
