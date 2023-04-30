#pragma once


#include <planet/audio/stereo.hpp>

#include <felspar/memory/small_vector.hpp>


namespace planet::audio {


    /// Can be given arbitrary input streams and produces output of the same format
    class mixer final {
      public:
        mixer() {}
        mixer(mixer const &) = delete;
        mixer(mixer &&) = delete;
        mixer &operator=(mixer const &) = delete;
        mixer &operator=(mixer &&) = delete;

        void add_track(stereo_generator track) {
            generators.push_back({std::move(track)});
        }

        stereo_generator output();

      private:
        struct channel {
            stereo_generator audio;
            /// The number of samples that have been placed in the output so far
            std::size_t samples = {};
        };
        std::vector<channel> generators;
    };


}
