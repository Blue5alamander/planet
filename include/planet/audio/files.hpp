#pragma once


#include <planet/audio/stereo.hpp>
#include <planet/serialise/forward.hpp>

#include <felspar/coro/generator.hpp>


namespace planet::audio {


    /// ## FLAC
    class flac final {
        struct impl;
        std::vector<std::byte> m_filedata;
        std::source_location loc;

      public:
        static std::string_view constexpr box{"_p:a:flac"};


        explicit flac(
                std::vector<std::byte> = {},
                std::source_location = std::source_location::current());


        /// ### Queries

        std::size_t channels() const;
        planet::audio::sample_clock duration() const;
        std::size_t sample_rate() const;

        std::span<std::byte const> filedata() const noexcept {
            return m_filedata;
        }


        /// ### Audio output
        felspar::coro::generator<stereo_buffer> stereo();
    };

    void save(planet::serialise::save_buffer &, flac const &);
    void load(planet::serialise::box &, flac &);


    /// ## Ogg
    class ogg final {
        struct impl;
        std::vector<std::byte> m_filedata;
        std::source_location loc;

      public:
        static std::string_view constexpr box{"_p:a:ogg"};


        explicit ogg(
                std::vector<std::byte> = {},
                std::source_location = std::source_location::current());


        /// ### Queries

        std::size_t channels() const;
        planet::audio::sample_clock duration() const;
        std::size_t sample_rate() const;

        std::span<std::byte const> filedata() const noexcept {
            return m_filedata;
        }


        /// ### Audio output
        felspar::coro::generator<stereo_buffer> stereo();
    };

    void save(planet::serialise::save_buffer &, ogg const &);
    void load(planet::serialise::box &, ogg &);


}
