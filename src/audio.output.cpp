#include <planet/audio/backend.hpp>
#include <planet/audio/mixer.hpp>
#include <planet/audio/output.hpp>
#include <planet/functional.hpp>
#include <planet/threading.hpp>

#include <felspar/exceptions/logic_error.hpp>


/// ## `planet::audio::backend`


planet::audio::backend::~backend() = default;


/// ## `planet::audio::output`


planet::audio::output::output(std::function<float()> master_gain) noexcept
: master_gain{std::move(master_gain)} {
    last_master_mul = this->master_gain();
}


void planet::audio::output::reserve_mixers(std::size_t const count) {
    mixers.reserve(count);
    last_underruns.resize(count, 0);
}


void planet::audio::output::attach(mixer &m) {
    std::scoped_lock lock{attach_mtx};
    if (mixers.size() == mixers.capacity()) {
        throw felspar::stdexcept::logic_error{
                "output::attach would grow the mixer vector past its reserved capacity while the audio callback may be reading it"};
    }
    m.bind_driver(*drv);
    m.begin();
    mixers.push_back(&m);
    attached.store(mixers.size(), std::memory_order_release);
}


void planet::audio::output::rebuild_driver(
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
     * Publish the negotiated block size so the generators size their look-ahead
     * to the real working block.
     */
    active_buffer_duration.store(
            sample_clock{static_cast<sample_clock::rep>(block_size)});
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


namespace {
    planet::telemetry::counter c_clip_count{"planet_audio__output__clip_count"};
    planet::telemetry::counter c_underrun_count{
            "planet_audio__output__underrun_count"};
}


void planet::audio::output::render(
        float *const output, std::size_t const wanted) noexcept {
    planet::threading::flush_denormals_to_zero();

    float const old_master_mul = last_master_mul;
    float const target_master_mul = master_gain();
    std::size_t const count = attached.load(std::memory_order_acquire);

    telemetry::counter::value_type clipped{};
    planet::by_index(wanted, [&](std::size_t const sample) {
        std::array<float, stereo_buffer::channels> mix = {};
        for (std::size_t m{}; m < count; ++m) {
            auto const frame = mixers[m]->next_frame();
            for (std::size_t ch{}; ch < stereo_buffer::channels; ++ch) {
                mix[ch] += frame[ch];
            }
        }
        float const master_mul = std::lerp(
                old_master_mul, target_master_mul,
                static_cast<float>(sample) / wanted);
        planet::by_index(
                stereo_buffer::channels, [&](std::size_t const channel) {
                    float const value = mix[channel] * master_mul;
                    output[sample * stereo_buffer::channels + channel] = value;
                    if (value > 1.0f or value < -1.0f) { ++clipped; }
                });
    });
    c_clip_count += clipped;
    last_master_mul = target_master_mul;

    /// Publish the per-mixer underrun deltas once per render.
    for (std::size_t m{}; m < count; ++m) {
        auto const total = mixers[m]->underrun_count();
        c_underrun_count += total - last_underruns[m];
        last_underruns[m] = total;
    }

    auto const advanced = drv->playback_head.load(std::memory_order_relaxed)
            + sample_clock{static_cast<sample_clock::rep>(wanted)};
    drv->playback_head.store(advanced, std::memory_order_release);
}
