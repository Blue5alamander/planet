#pragma once


#include <planet/audio/buffer.hpp>
#include <planet/audio/clocks.hpp>

#include <felspar/coro/generator.hpp>
#include <felspar/memory/accumulation_buffer.hpp>


namespace planet::audio {


    /// ## Stereo output at our default audio sample rate
    using stereo_buffer =
            planet::audio::buffer_storage<planet::audio::sample_clock, 2>;


    /// ## Stereo audio source
    using stereo_generator = felspar::coro::generator<stereo_buffer>;


    /// Converts a mono buffer to a stereo buffer
    template<typename Clock>
    inline felspar::coro::generator<stereo_buffer> stereobuffer(
            felspar::coro::generator<buffer_storage<Clock, 1>> mono) {
        felspar::memory::accumulation_buffer<float> output{
                default_buffer_samples * 50};
        for (auto block : mono) {
            output.ensure_length(block.samples() * stereo_buffer::channels);
            for (std::size_t index{}; index < block.samples(); ++index) {
                output[index * stereo_buffer::channels + 0] = block[index][0];
                output[index * stereo_buffer::channels + 1] = block[index][0];
            }
            co_yield output.first(block.samples() * stereo_buffer::channels);
        }
    }


}
