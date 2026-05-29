#pragma once


#include <planet/audio/stereo.hpp>
#include <planet/queue/tspsc.hpp>

#include <felspar/memory/small_vector.hpp>


namespace planet::audio {


    /// ## Audio mixer
    /// Can be given arbitrary input streams and produces output of the same format
    class mixer final {
      public:
        mixer(channel &c) : master{c} {
            incoming.reserve(generators.capacity());
        }

        mixer(mixer const &) = delete;
        mixer(mixer &&) = delete;
        mixer &operator=(mixer const &) = delete;
        mixer &operator=(mixer &&) = delete;


        void add_track(stereo_generator track) {
            incoming.push(std::move(track));
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
        /// Tracks waiting to join the mix. `add_track` pushes from any thread;
        /// `raw_mix` drains this into `generators` at the start of each buffer,
        /// keeping `generators` itself audio-thread-only (no lock needed).
        planet::queue::tspsc<stereo_generator> incoming;
        felspar::memory::small_vector<track, 50> generators;
        stereo_generator raw_mix();
    };


}
