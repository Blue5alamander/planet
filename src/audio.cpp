#include <planet/audio/gain.hpp>
#include <planet/audio/mixer.hpp>
#include <planet/audio/music.hpp>
#include <planet/audio/oscillator.hpp>
#include <planet/numbers.hpp>

#include <felspar/memory/accumulation_buffer.hpp>

#include <cmath>
#include <complex>


/// ## `planet::audio::atomic_linear_gain`


void planet::audio::atomic_linear_gain::set(linear_gain const g) {
    multiplier = g.multiplier;
}


/// ## `planet::audio::dB_gain`


planet::audio::dB_gain::dB_gain(float const g) : dB{g} {}


/// ## `planet::audio::linear_gain`


planet::audio::linear_gain::linear_gain(float const g) : multiplier{g} {}


/// ## `planet::audio::mixer`


auto planet::audio::mixer::output() -> stereo_generator {
    felspar::memory::accumulation_buffer<float> output{
            default_buffer_samples * 50};
    while (true) {
        std::erase_if(generators, [&output](auto &gen) {
            while (gen.samples < default_buffer_samples) {
                auto buffer = gen.audio.next();
                if (buffer) {
                    output.ensure_length(
                            (gen.samples + buffer->samples())
                            * stereo_buffer::channels);
                    for (std::size_t sample{}; sample < buffer->samples();
                         ++sample) {
                        std::size_t const idx = (gen.samples + sample)
                                * stereo_buffer::channels;
                        auto const src = (*buffer)[sample];
                        output.at(idx + 0) += src[0];
                        output.at(idx + 1) += src[1];
                    }
                    gen.samples += buffer->samples();
                } else {
                    return true;
                }
            }
            gen.samples -= default_buffer_samples;
            return false;
        });
        output.ensure_length(default_buffer_samples * stereo_buffer::channels);
        co_yield output.first(default_buffer_samples * stereo_buffer::channels);
    }
}


/// ## `planet::audio::music`


planet::audio::stereo_generator planet::audio::music::output() {
    felspar::memory::accumulation_buffer<float> output{
            default_buffer_samples * 50};
    std::optional<planet::audio::stereo_generator> playing;
    while (true) {
        if (clear_flag.load(std::memory_order_relaxed)) {
            clear_flag.store(false, std::memory_order_relaxed);
            playing = {};
            std::scoped_lock _{mtx};
            queue.clear();
        }
        if (not playing) {
            std::scoped_lock _{mtx};
            if (queue.size()) { playing = queue.front().start(); }
        }
        if (playing) {
            while (auto block = playing->next()) {
                if (clear_flag.load(std::memory_order_relaxed)) {
                    /// TODO Micro fade block
                    // co_yield block;
                    break;
                } else {
                    co_yield *block;
                }
            }
            playing = {};
        } else {
            output.ensure_length(
                    default_buffer_samples * stereo_buffer::channels);
            co_yield output.first(
                    default_buffer_samples * stereo_buffer::channels);
        }
    }
}


felspar::coro::task<void>
        planet::audio::music::clear(felspar::io::warden &ward) {
    clear_flag.store(true, std::memory_order_relaxed);
    while (clear_flag.load(std::memory_order_relaxed)) {
        co_await ward.sleep(50ms);
    }
}


void planet::audio::music::enqueue(start_tune_function tn) {
    std::scoped_lock _{mtx};
    queue.push_back({std::move(tn)});
}


void planet::audio::music::set_volume(dB_gain const dB) {
    master = dB;
    master_gain.set(static_cast<linear_gain>(master));
}


/// ## `planet::audio::oscillator`


felspar::coro::generator<std::span<float>> silence() {
    std::array<float, planet::audio::default_buffer_samples> buffer{};
    while (true) { co_yield buffer; }
}


felspar::coro::generator<std::span<float>>
        planet::audio::oscillator(float const turns) {
    std::array<float, planet::audio::default_buffer_samples> buffer;
    std::complex const rotate{std::cos(turns * τ), std::sin(turns * τ)};
    std::complex phase{1.0f, 0.0f};
    while (true) {
        for (auto &s : buffer) {
            s = phase.imag();
            phase *= rotate;
        }
        co_yield buffer;
    }
}
