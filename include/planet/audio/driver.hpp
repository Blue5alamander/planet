#pragma once


#include <planet/audio/clocks.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>


namespace planet::audio {


    /// ## Audio backend driver parameters
    /**
     * Single read-only handle that an audio backend (e.g.
     * `planet::sdl::audio_output`) hands to each attached mixer. The backend
     * owns one `driver` instance, writes to `playback_head` from its real-time
     * thread, and exposes the rest as immutable timing/sizing parameters that
     * the mixer needs to size its ring and find the audio clock.
     */
    struct driver final {
        driver(std::size_t block_size, std::size_t block_count) noexcept;

        driver(driver const &) = delete;
        driver(driver &&) = delete;
        driver &operator=(driver const &) = delete;
        driver &operator=(driver &&) = delete;

        /// ### Playback head
        /**
         * Audio-clock position the backend will have written through by the
         * end of the next block it produces. Written by the backend's
         * real-time callback; read lock-free by anything holding a `driver`.
         */
        std::atomic<sample_clock> playback_head{};

        /// ### Samples per buffer block
        std::size_t const block_size;

        /// ### Blocks of buffered latency (== mixer ring depth)
        std::size_t const block_count;

        /// ### Latency in samples (== block_size * block_count)
        sample_clock const latency;

        /// ### Wall-clock anchor for sample-position zero
        /**
         * The real-world (`steady_clock`) instant that corresponds to the
         * mixer producer's first output sample. Captured **once** when the
         * driver is constructed — i.e. once per `reconnect`, because the
         * backend re-emplaces the driver each time it renegotiates a device —
         * and never written again. Holding the audio↔wall offset fixed for the
         * whole session means a given wall-clock `play_at` always maps to the
         * same audio sample: no per-callback rounding can make successive calls
         * disagree about which sample a time represents.
         *
         * `mixer::add_track(track, play_at)` reads this to convert a scheduled
         * wall-clock time into an absolute producer-sample position. It is a
         * plain `const` member (not atomic): written before the driver pointer
         * is published to any mixer, then only ever read.
         */
        std::chrono::steady_clock::time_point const wall_clock_epoch;
    };


}
