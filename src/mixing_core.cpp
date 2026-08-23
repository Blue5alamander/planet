#include <planet/audio/backend.hpp>
#include <planet/audio/mixer.hpp>
#include <planet/audio/mixing_core.hpp>
#include <planet/functional.hpp>

#include <felspar/exceptions/logic_error.hpp>


/// ## `planet::audio::backend`


planet::audio::backend::~backend() = default;


/// ## `planet::audio::mixing_core`


planet::audio::mixing_core::mixing_core(
        std::function<float()> master_gain) noexcept
: master_gain{std::move(master_gain)} {}


void planet::audio::mixing_core::reserve_mixers(std::size_t const count) {
    mixers.reserve(count);
}


void planet::audio::mixing_core::attach(mixer &m) {
    std::scoped_lock lock{attach_mtx};
    if (mixers.size() == mixers.capacity()) {
        /**
         * `push_back` here would reallocate and move the buffer the audio
         * callback reads lock-free on the real-time thread. Every backend
         * calls `reserve_mixers` before attaching, so reaching this means
         * `attach` was called without reserving room — a programming error.
         */
        throw felspar::stdexcept::logic_error{
                "mixing_core::attach would grow the mixer vector past its "
                "reserved capacity while the audio callback may be reading it"};
    }
    m.bind_driver(*drv);
    m.begin();
    mixers.push_back(&m);
    attached.store(mixers.size(), std::memory_order_release);
}


void planet::audio::mixing_core::rebuild_driver(
        std::size_t const block_size, std::size_t const block_count) {
    drv.emplace(block_size, block_count);
    /**
     * Seed the published playback head with the end-time of the first block,
     * so a producer thread that polls the clock before the first callback fires
     * sees the same kind of value it will see between subsequent callbacks.
     */
    drv->playback_head.store(
            sample_clock{static_cast<sample_clock::rep>(block_size)},
            std::memory_order_release);
    /**
     * Publish the negotiated block size so the generators below `planet-sdl`
     * size their look-ahead to the real working block. Centralised here so
     * every backend does it from its own negotiated size rather than the SDL
     * device alone doing it from the config value.
     */
    active_buffer_duration.store(
            sample_clock{static_cast<sample_clock::rep>(block_size)});
    /**
     * Rebind every mixer attached before this rebuild. The device just
     * renegotiated its block size, so those mixers still hold a driver pointer
     * to the storage just re-emplaced and would render slots sized for the
     * previous block. `bind_driver` stops a running producer, resets the ring,
     * and pre-rolls silence; `begin()` restarts it. Newly-attached mixers are
     * handled by `attach` itself.
     */
    {
        std::scoped_lock lock{attach_mtx};
        std::size_t const count = attached.load(std::memory_order_acquire);
        planet::by_index(count, [&](auto const i) {
            mixers[i]->bind_driver(*drv);
            mixers[i]->reset_schedule_margin_min();
            mixers[i]->begin();
        });
    }
    mixer::reset_worst_schedule_margin_min();
}
