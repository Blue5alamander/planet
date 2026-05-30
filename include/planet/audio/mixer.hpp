#pragma once


#include <planet/audio/clocks.hpp>
#include <planet/audio/stereo.hpp>
#include <planet/functional.hpp>
#include <planet/queue/tspsc.hpp>

#include <felspar/memory/small_vector.hpp>

#include <array>
#include <atomic>
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
    class mixer final {
      public:
        /// ### Maximum pre-render ring depth
        /**
         * Compile-time cap on the number of blocks buffered between the
         * producer thread and the consuming audio callback. The depth actually
         * used is derived from the latency the audio output passes to
         * `bind_playback_clock`; this only bounds the backing storage and the
         * semaphore.
         */
        static constexpr std::size_t max_ring_depth = 16;


        /// ### Construction
        explicit mixer(channel &c);
        /**
         * The mixer is unattached until `bind_playback_clock` is called: that
         * is the step that sets the ring depth (from a latency supplied by the
         * audio output) and pre-rolls silence. `begin` must therefore be
         * preceded by `bind_playback_clock`, which `audio_output::attach` does.
         */

        mixer(mixer const &) = delete;
        mixer(mixer &&) = delete;
        mixer &operator=(mixer const &) = delete;
        mixer &operator=(mixer &&) = delete;

        ~mixer();


        /// ### Add a track to the mix
        /// Safe to call from any thread; drained on the producer thread.
        void add_track(stereo_generator track) {
            incoming.push(std::move(track));
        }
        void add_track(stereo_generator track, dB_gain g) {
            add_track(gain(g, std::move(track)));
        }

        /// ### Mixed output generator
        stereo_generator output();
        /// Single consumer: pulled by the producer thread once `begin` is called.


        /// ### Start the producer thread
        void begin();
        /**
         * Launches the producer thread. The ring was declared full of silence
         * by `bind_playback_clock`, so the audio callback may begin pulling
         * from `next_frame` immediately — every `add_track` therefore becomes
         * audible exactly `latency` later, with no startup window in which
         * that promise can be undercut. The producer wakes only as the
         * callback frees ring slots, so it stays bounded at `depth` blocks of
         * lead. Call exactly once, after `bind_playback_clock`, when the
         * mixer is attached to an audio output.
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
            if (++read_marker == default_buffer_samples) {
                read_marker = 0;
                read_slot = (read_slot + 1) % depth;
                ready_count.fetch_sub(1, std::memory_order_acq_rel);
                slots_free.release();
            }
            return frame;
        }


        /// ### Number of underruns observed (telemetry)
        std::uint64_t underrun_count() const noexcept {
            return underruns.load(std::memory_order_relaxed);
        }


        /// ### Bind the SDL playback-head clock and configure the ring
        /**
         * Called by `planet::sdl::audio_output::attach` before the producer
         * thread starts. Does three things:
         *
         * 1. Binds the atomic published by the `audio_output`'s callback so
         *    anything holding only the mixer can find the shared audio-clock
         *    value (`playback_clock()`).
         * 2. Sets the producer's bounded lead from `latency` — the fixed
         *    distance the producer can stay ahead of the consuming callback.
         *    A track handed to `add_track` therefore becomes audible
         *    `latency` later (give or take one block), measured on top of
         *    the audio device's own buffer.
         * 3. Pre-rolls the ring with silence so the callback can begin
         *    consuming immediately, without a startup window in which the
         *    fixed-latency promise could be undercut.
         */
        void bind_playback_clock(
                std::atomic<sample_clock> const &c,
                sample_clock latency) noexcept;

        /// ### SDL playback-head clock, or `nullptr` if not yet bound
        /**
         * Returns a pointer to the atomic published by `audio_output`'s
         * callback (end-time of the next block SDL will play). Pointer
         * rather than reference so an unattached mixer (e.g. in tests) is
         * representable.
         */
        std::atomic<sample_clock> const *playback_clock() const noexcept {
            return playback;
        }


        /// ### Bounded producer lead in blocks (derived from `latency`)
        std::size_t buffer_depth() const noexcept { return depth; }

        /// ### Blocks currently rendered and ready for the callback (telemetry)
        std::size_t buffered_blocks() const noexcept {
            return static_cast<std::size_t>(
                    ready_count.load(std::memory_order_relaxed));
        }


      private:
        channel &master;
        struct track {
            stereo_generator audio;
            /// The number of samples that have been placed in the output so far
            std::size_t samples = {};
        };
        /**
         * Tracks waiting to join the mix. `add_track` pushes from any thread;
         * `raw_mix` drains this into `generators` at the start of each buffer,
         * keeping `generators` itself producer-thread-only (no lock needed).
         */
        planet::queue::tspsc<stereo_generator> incoming;
        felspar::memory::small_vector<track, 50> generators;
        stereo_generator raw_mix();

        /**
         * Bounded producer lead in blocks, derived from the latency the audio
         * output passes to `bind_playback_clock`. The producer can be at most
         * this many blocks ahead of the consumer, so the realized latency is
         * fixed at `depth` blocks rather than the whole backing ring. Zero
         * until `bind_playback_clock` is called.
         */
        std::size_t depth = 0;

        /**
         * Pre-rendered ring shared with the audio callback. Backed by the
         * compile-time cap; only the first `depth` slots are ever used. Each
         * slot holds the latest published block as a refcounted
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
        std::atomic<sample_clock> const *playback = nullptr;
    };


}
