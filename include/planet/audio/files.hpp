#pragma once


#include <planet/audio/buffer.hpp>
#include <planet/audio/clocks.hpp>

#include <felspar/coro/generator.hpp>


namespace planet::audio {


    /// ## Load an ogg file
    class ogg final {
        struct impl;
        std::vector<std::byte> filedata;
        felspar::source_location loc;

      public:
        explicit ogg(
                std::vector<std::byte>,
                felspar::source_location const & =
                        felspar::source_location::current());

        felspar::coro::generator<
                planet::audio::buffer_storage<planet::audio::sample_clock, 2>>
                stereo();
    };


    /// ## Load a WAV file
    class wav final {
        buffer_storage<sample_clock, 2> samples;

      public:
        explicit wav(std::span<std::byte const>);
        explicit wav(std::vector<std::byte> const &v)
        : wav{std::span{v.data(), v.size()}} {}
        wav(wav const &) = delete;
        wav(wav &&) = delete;
        wav &operator=(wav const &) = delete;
        wav &operator=(wav &&) = delete;

        /// Produce output in small chunks up to the `default_buffer_duration`
        felspar::coro::generator<buffer_storage<sample_clock, 2>> output();
    };


}
