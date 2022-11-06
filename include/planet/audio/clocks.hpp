#pragma once


#include <chrono>


namespace planet::audio {


    /// Standard audio output sample rate
    using sample_clock =
            std::chrono::duration<std::int64_t, std::ratio<1, 48'000>>;


}
