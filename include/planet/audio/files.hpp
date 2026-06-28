#pragma once


#include <planet/audio/stereo.hpp>

#include <felspar/coro/generator.hpp>


namespace planet::audio {


    /// ## FLAC
    class flac final {
        struct impl;
        std::vector<std::byte> m_filedata;
        std::source_location loc;

      public:
        explicit flac(
                std::vector<std::byte>,
                std::source_location = std::source_location::current());


        planet::audio::sample_clock duration() const;
        felspar::coro::generator<stereo_buffer> stereo();


        std::span<std::byte const> filedata() const noexcept {
            return m_filedata;
        }
    };


    /// ## Ogg
    class ogg final {
        struct impl;
        std::vector<std::byte> m_filedata;
        std::source_location loc;

      public:
        explicit ogg(
                std::vector<std::byte>,
                std::source_location = std::source_location::current());


        planet::audio::sample_clock duration() const;
        felspar::coro::generator<stereo_buffer> stereo();


        std::span<std::byte const> filedata() const noexcept {
            return m_filedata;
        }
    };


}
