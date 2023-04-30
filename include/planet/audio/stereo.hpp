#pragma once


#include <planet/audio/buffer.hpp>
#include <planet/audio/clocks.hpp>

#include <felspar/coro/generator.hpp>


namespace planet::audio {


    /// ## Stereo output at our default audio sample rate
    using stereo_buffer =
            planet::audio::buffer_storage<planet::audio::sample_clock, 2>;


    /// ## Stereo audio source
    using stereo_generator = felspar::coro::generator<stereo_buffer>;


}
