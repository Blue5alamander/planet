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
         * The `steady_clock` instant of the producer's output sample zero, read
         * by `mixer::add_track(track, play_at)` to place a scheduled time on
         * the producer timeline. Seeded at construction, then re-anchored once
         * by `mixer::raw_mix` when the device has settled: construction runs
         * before the device wakes, so the seed alone would fold the warm-up gap
         * into every scheduled position. `mutable`/atomic because that
         * re-anchor runs on the producer thread while `add_track` reads from
         * any thread.
         */
        mutable std::atomic<std::chrono::steady_clock::time_point>
                wall_clock_epoch;
    };


}
