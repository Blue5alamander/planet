#pragma once


#include <planet/audio/clocks.hpp>
#include <planet/audio/driver.hpp>
#include <planet/audio/stereo.hpp>
#include <planet/functional.hpp>
#include <planet/queue/tspsc.hpp>
#include <planet/telemetry/counter.hpp>
#include <planet/telemetry/id.hpp>

#include <felspar/memory/small_vector.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <semaphore>
#include <thread>


namespace planet::audio {


    /// ## Audio mixer
    /**
     * Mixes arbitrary input streams and produces output of the same format.
     *
     * The mix is produced on the mixer's **own thread** into a small ring of
     * pre-rendered blocks. The audio output's real-time callback consumes ready
     * blocks through `next_frame` (which never blocks) and wakes the producer
     * thread by freeing a ring slot, so all synth/mix work happens off the
     * real-time thread.
     */
    class mixer final : private telemetry::id {
      public:
        /// ### Maximum pre-render ring depth
        /**
         * Compile-time cap on the number of blocks buffered between the
         * producer thread and the consuming audio callback. The depth actually
         * used is `driver::block_count`, taken from the `driver` the audio
         * output passes to `bind_driver`; this only bounds the backing storage
         * and the semaphore.
         */
        static constexpr std::size_t max_ring_depth = 16;


        /// ### Construction
        explicit mixer(channel &c);
        /// Named overload: `name` becomes the base for this mixer's telemetry.
        mixer(std::string_view name, channel &c);
        /**
         * The mixer is unattached until `bind_driver` is called: that is the
         * step that sets the ring depth (from the driver supplied by the audio
         * output) and pre-rolls silence. `begin` must therefore be preceded by
         * `bind_driver`, which `audio_output::attach` does.
         *
         * For normal use prefer the named overload: the supplied `name` (made
         * unique by `telemetry::id`) is the base for this mixer's per-instance
         * telemetry counters, so they can be told apart in the registry. The
         * unnamed overload still produces a unique machine-generated name and
         * is intended for tests and tools.
         */

        mixer(mixer const &) = delete;
        mixer(mixer &&) = delete;
        mixer &operator=(mixer const &) = delete;
        mixer &operator=(mixer &&) = delete;

        ~mixer();


        /// ### Add a track to the mix
        /// Safe to call from any thread; drained on the producer thread.
        /**
         * The track starts as soon as the producer next drains the queue —
         * audible `driver::latency` later, the fixed-latency promise the mixer
         * is built on. Pushes a `sample_clock::min()` sentinel so the producer
         * applies no scheduling delay at all (exactly the historical
         * behaviour), independent of whether a driver is bound.
         */
        void add_track(stereo_generator track) {
            incoming.push({std::move(track), sample_clock::min()});
        }
        void add_track(stereo_generator track, dB_gain g) {
            add_track(gain(g, std::move(track)));
        }

        /// ### Add a track scheduled to start at a real-world wall-clock time
        /**
         * `play_at` is a `steady_clock` instant. The track is converted to an
         * absolute producer-sample position using the driver's fixed
         * `wall_clock_epoch` (captured once per `reconnect`), so the same
         * `play_at` always lands on the same sample for the whole session. The
         * producer outputs silence until that position, then mixes the track
         * in.
         *
         * The target is pinned a fixed `driver::latency` ahead of the
         * producer's write head, so a `play_at` is realised at a fixed,
         * predictable offset rather than "as soon as possible": the start does
         * not drift with capture-to-queue processing time, and two times
         * scheduled from the same captured instant keep their exact relative
         * spacing. If `play_at` is far enough in the past that even with this
         * headroom it falls behind the producer's write position, the track
         * starts as soon as possible (no negative delay). On a mixer with no
         * driver bound the time cannot be resolved, so the track starts as soon
         * as possible.
         */
        void add_track(
                stereo_generator track,
                std::chrono::steady_clock::time_point play_at);
        void add_track(
                stereo_generator track,
                dB_gain g,
                std::chrono::steady_clock::time_point const play_at) {
            add_track(gain(g, std::move(track)), play_at);
        }

        /// ### Mixed output generator
        stereo_generator output();
        /// Single consumer: pulled by the producer thread once `begin` is called.


        /// ### Start the producer thread
        void begin();
        /**
         * Launches the producer thread. The ring was declared full of silence
         * by `bind_driver`, so the audio callback may begin pulling from
         * `next_frame` immediately — every `add_track` therefore becomes
         * audible exactly `driver::latency` later, with no startup window in
         * which that promise can be undercut. The producer wakes only as the
         * callback frees ring slots, so it stays bounded at
         * `driver::block_count` blocks of lead. Call exactly once, after
         * `bind_driver`, when the mixer is attached to an audio output.
         */


        /// ### Next stereo frame for the audio callback
        std::array<float, stereo_buffer::channels> next_frame() noexcept
        /**
         * Returns the next `{left, right}` frame from the ring, or silence (and
         * counts an underrun) when no block is ready. Frees a ring slot —
         * waking the producer — each time a whole block has been consumed.
         * Never blocks. Only ever called from the single audio-callback thread.
         *
         * Reads through the slot's `shared_buffer<float>` handle without
         * touching its refcount: the producer thread owns the slot, and the
         * callback only reads via `.data()`. The slot's handle is overwritten
         * (with the next published block) only on the producer thread, after
         * `slots_free.release()` here gives it permission.
         */
        {
            if (read_marker == 0
                and ready_count.load(std::memory_order_acquire) == 0) {
                underruns.fetch_add(1, std::memory_order_relaxed);
                return {};
            }
            auto const base = read_marker * stereo_buffer::channels;
            float const *const samples = slots[read_slot].data();
            std::array<float, stereo_buffer::channels> frame;
            planet::by_index(
                    stereo_buffer::channels, [&](std::size_t const ch) {
                        frame[ch] = samples[base + ch];
                    });
            if (++read_marker == drv->block_size) {
                read_marker = 0;
                read_slot = (read_slot + 1) % drv->block_count;
                ready_count.fetch_sub(1, std::memory_order_acq_rel);
                slots_free.release();
            }
            return frame;
        }


        /// ### Number of underruns observed (telemetry)
        std::uint64_t underrun_count() const noexcept {
            return underruns.load(std::memory_order_relaxed);
        }

        /// ### Scheduled tracks that had to start "as soon as possible"
        /**
         * Count of tracks given an explicit `play_at` whose target had already
         * fallen behind the producer's write head by the time it was drained —
         * i.e. the fixed `driver::latency` headroom was not enough to absorb
         * the capture-to-queue delay, so the track started immediately instead
         * of at its scheduled position. For a producer that always schedules
         * valid future times a non-zero value means the latency is too short.
         * Does not count the immediate `add_track` overload (which is ASAP by
         * design).
         */
        telemetry::counter::value_type asap_scheduled_count() const noexcept {
            return asap_scheduled.value();
        }


        /// ### Bind the audio driver and configure the ring
        /**
         * Called by `planet::sdl::audio_output::attach` before the producer
         * thread starts, and again by `audio_output::reconnect` whenever a
         * new device is negotiated and a fresh `driver` is constructed.
         * Does three things:
         *
         * 1. Stores the driver pointer so anything holding only the mixer
         *    can find the shared audio-clock value (`playback_clock()`) and
         *    the backend's timing/sizing parameters.
         * 2. Sets the producer's bounded lead to `driver::block_count` — the
         *    fixed distance the producer can stay ahead of the consuming
         *    callback. A track handed to `add_track` therefore becomes
         *    audible `driver::latency` later (give or take one block),
         *    measured on top of the audio device's own buffer.
         * 3. Pre-rolls the ring with silence so the callback can begin
         *    consuming immediately, without a startup window in which the
         *    fixed-latency promise could be undercut.
         *
         * Safe to call more than once: if a producer thread is already
         * running it is stopped and joined first, the ring's
         * producer/consumer state is reset, and the slot buffers are
         * re-allocated for the new driver's `block_size`. The audio
         * callback must be quiesced (e.g. the SDL device closed or paused)
         * around the call so the consumer side does not observe the reset
         * mid-flight. Call `begin()` afterwards to restart the producer.
         */
        void bind_driver(driver const &) noexcept;

        /// ### SDL playback-head clock, or `nullptr` if not yet bound
        /**
         * Returns a pointer to the atomic published by `audio_output`'s
         * callback (end-time of the next block SDL will play). Pointer
         * rather than reference so an unattached mixer (e.g. in tests) is
         * representable.
         */
        std::atomic<sample_clock> const *playback_clock() const noexcept {
            return drv ? &drv->playback_head : nullptr;
        }


        /// ### Bounded producer lead in blocks (== `driver::block_count`)
        std::size_t buffer_depth() const noexcept {
            return drv ? drv->block_count : 0;
        }

        /// ### Blocks currently rendered and ready for the callback (telemetry)
        std::size_t buffered_blocks() const noexcept {
            return static_cast<std::size_t>(
                    ready_count.load(std::memory_order_relaxed));
        }


      private:
        channel &master;
        /// Incremented (and propagated to a global parent) whenever a scheduled
        /// track is clamped to "as soon as possible"; see `asap_scheduled_count`.
        telemetry::counter asap_scheduled;
        struct track {
            stereo_generator audio;
            /// The number of samples that have been placed in the output so far
            std::size_t samples = {};
            /// Silence still to be written before this track starts mixing in
            std::size_t delay_samples = {};
        };
        /**
         * A track waiting to join the mix together with the absolute
         * producer-sample position it should start at. `add_track` resolves
         * `play_at` into `target_position`; a `sample_clock::min()` value means
         * "as soon as possible" (no scheduling delay). `raw_mix` turns the
         * target into `track::delay_samples` against its own write position.
         */
        struct scheduled_track {
            stereo_generator audio;
            sample_clock target_position;
        };
        /**
         * Tracks waiting to join the mix. `add_track` pushes from any thread;
         * `raw_mix` drains this into `generators` at the start of each buffer,
         * keeping `generators` itself producer-thread-only (no lock needed).
         */
        planet::queue::tspsc<scheduled_track> incoming;
        felspar::memory::small_vector<track, 50> generators;
        stereo_generator raw_mix();

        /**
         * Pre-rendered ring shared with the audio callback. Backed by the
         * compile-time cap; only the first `drv->block_count` slots are ever
         * used. Each slot holds the latest published block as a refcounted
         * `shared_buffer<float>` slice from the producer's accumulation buffer;
         * future tap subscribers can hold their own ref on the same slice
         * without an extra copy. Initialized to zero-filled buffers in the
         * constructor so the callback sees pre-rolled silence before the
         * producer thread starts.
         */
        using slot = felspar::memory::shared_buffer<float>;
        std::array<slot, max_ring_depth> slots = {};
        std::atomic<int> ready_count = 0;
        std::counting_semaphore<max_ring_depth> slots_free;

        /// Producer thread and its loop
        std::atomic<bool> stop_flag = false;
        std::thread producer;
        std::size_t write_slot = 0;
        void run() noexcept;

        /// Audio-callback-thread-only consumer state
        std::size_t read_slot = 0;
        std::size_t read_marker = 0;
        std::atomic<std::uint64_t> underruns = 0;

        /// Bound by `audio_output::attach`; null until then.
        driver const *drv = nullptr;
    };


}
