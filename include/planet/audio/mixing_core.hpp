#pragma once


#include <planet/audio/clocks.hpp>
#include <planet/audio/driver.hpp>
#include <planet/audio/forward.hpp>

#include <functional>
#include <mutex>
#include <vector>


namespace planet::audio {


    /// ## Shared audio output mixing core
    /**
     * Takes the mixers and the master gain and mixes into the blocks for
     * delivery to the backend.
     */
    class mixing_core final {
      public:
        /// ### Construction
        explicit mixing_core(std::function<float()> master_gain) noexcept;


        /// ### Reserve storage for the mixers attached at construction
        void reserve_mixers(std::size_t);
        /**
         * Fixes the capacity of the mixer vector before the first `attach`, so
         * `attach`'s `push_back` never reallocates the buffer the real-time
         * callback reads lock-free.
         */

        /// ### Attach a mixer
        void attach(mixer &);
        /**
         * Binds the mixer to the current driver, starts its producer thread,
         * and registers it so the callback sums its output. Throws if it would
         * grow the mixer vector past the reserved capacity. The mixer must
         * outlive this core.
         */

        /// ### (Re)build the driver for a negotiated block size
        void rebuild_driver(std::size_t block_size, std::size_t block_count);
        /**
         * Emplaces a fresh driver for the block size the device just
         * negotiated, seeds the playback head to the end of the first block,
         * publishes the block size into `active_buffer_duration` so producers
         * below `planet-sdl` size their look-ahead to the real working block,
         * and rebinds every already-attached mixer to the new driver. The
         * caller must have quiesced the callback (device closed or paused)
         * across the call.
         */


        /// ### Playback head
        std::atomic<sample_clock> const &playback_clock() const noexcept {
            return drv->playback_head;
        }


      private:
        std::function<float()> master_gain;

        /// ### Backend driver, engaged by the first `rebuild_driver`
        std::optional<driver> drv;

        /**
         * Attached mixers, written only by `attach`/`rebuild_driver` under
         * `attach_mtx` and published via `attached`; read lock-free by the
         * callback. Reserved to the final count by `reserve_mixers` so `attach`
         * never reallocates under the live callback.
         */
        std::vector<mixer *> mixers;
        std::atomic<std::size_t> attached = 0;
        std::mutex attach_mtx;
    };


}
