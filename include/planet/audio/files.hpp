#pragma once


#include <planet/audio/stereo.hpp>

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

        felspar::coro::generator<stereo_buffer> stereo();
    };


}
