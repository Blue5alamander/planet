#pragma once


#include <planet/queue/tspsc.hpp>

#include <felspar/memory/shared_buffer.hpp>


namespace planet::audio {


    /// ## Mixer output tap
    /**
     * A thread-safe, single-consumer subscriber that receives a copy of every
     * block a `mixer` publishes, so the mixed stereo stream can be recorded,
     * shown on screen, or processed further off the mixer's thread.
     *
     * The mixer's **producer thread** calls `publish` once per rendered block;
     * a single **consumer thread** drains them with `consume`. Forwarding is a
     * refcount bump on the producer's `shared_buffer<float>` slice — no audio
     * data is copied, and holding the ref keeps that slice alive in the
     * mixer's accumulation buffer until the consumer drops it.
     *
     * The taps a mixer forwards to are fixed for the mixer's lifetime (a span
     * is passed to the mixer constructor), so a tap is wired up before the
     * producer thread starts and is non-copyable/non-movable: pass it around
     * by pointer or reference.
     *
     * The consumer must `consume` regularly: undrained blocks accumulate in
     * the queue and pin the backing audio buffers. A consumer that only wants
     * the most recent output (a scope, a level meter) should still drain the
     * whole span each time and use the last block.
     */
    class tap final {
      public:
        /// ### A single published block
        /**
         * One block of interleaved stereo samples (`{l, r, l, r, ...}`,
         * `mixer`'s `driver::block_size` frames long), refcounted so the
         * consumer can hold it without copying.
         */
        using block = felspar::memory::shared_buffer<float>;


        tap() = default;
        tap(tap const &) = delete;
        tap(tap &&) = delete;
        tap &operator=(tap const &) = delete;
        tap &operator=(tap &&) = delete;


        /// ### Pre-allocate queue capacity
        /**
         * Reserve room for `n` undrained blocks so the producer's `publish`
         * does not allocate in steady state. Call before the mixer's producer
         * thread starts.
         */
        void reserve(std::size_t const n) { blocks.reserve(n); }


        /// ### Forward a block — producer side
        /// Called on the mixer's producer thread only.
        void publish(block b) { blocks.push(std::move(b)); }


        /// ### Drain pending blocks — consumer side
        /**
         * Returns every block published since the last call, oldest first. The
         * returned span is valid until the next `consume`. Called on the single
         * consumer thread only.
         */
        std::span<block> consume() { return blocks.consume(); }


      private:
        planet::queue::tspsc<block> blocks;
    };


}
