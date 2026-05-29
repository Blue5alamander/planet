#pragma once


#include <planet/audio/stereo.hpp>

#include <felspar/memory/small_vector.hpp>

#include <mutex>


namespace planet::audio {


    /// ## Audio mixer
    /// Can be given arbitrary input streams and produces output of the same format
    class mixer final {
      public:
        mixer(channel &c) : master{c} {}

        mixer(mixer const &) = delete;
        mixer(mixer &&) = delete;
        mixer &operator=(mixer const &) = delete;
        mixer &operator=(mixer &&) = delete;


        void add_track(stereo_generator track) {
            std::scoped_lock _{mtx};
            if (generators.has_room()) {
                generators.push_back({std::move(track)});
            }
        }
        void add_track(stereo_generator track, dB_gain g) {
            add_track(gain(g, std::move(track)));
        }

        stereo_generator output();


      private:
        channel &master;
        struct track {
            stereo_generator audio;
            /// The number of samples that have been placed in the output so far
            std::size_t samples = {};
        };
        /// Guards `generators`, which is written from caller threads via
        /// `add_track` and read/restructured from the audio thread in `raw_mix`.
        std::mutex mtx;
        felspar::memory::small_vector<track, 50> generators;
        stereo_generator raw_mix();
    };


}
