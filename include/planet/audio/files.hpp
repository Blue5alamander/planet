#pragma once


#include <planet/audio/buffer.hpp>
#include <planet/audio/clocks.hpp>

#include <felspar/coro/generator.hpp>


namespace planet::audio {


    /// Load a WAV file
    class wav final {
        buffer_storage<sample_clock, 2> samples;

      public:
        wav(std::span<std::byte>);
        wav(wav const &) = delete;
        wav(wav &&) = delete;
        wav &operator=(wav const &) = delete;
        wav &operator=(wav &&) = delete;

        /// Produce output in small chunks up to the `default_buffer_duration`
        felspar::coro::generator<buffer_storage<sample_clock, 2>> output();
    };


}
