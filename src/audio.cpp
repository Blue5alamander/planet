#include <planet/audio/gain.hpp>
#include <planet/audio/mixer.hpp>
#include <planet/audio/music.hpp>
#include <planet/audio/oscillator.hpp>
#include <planet/log.hpp>
#include <planet/serialise.hpp>
#include <planet/numbers.hpp>

#include <felspar/memory/accumulation_buffer.hpp>

#include <cmath>
#include <complex>


/// ## `planet::audio::atomic_linear_gain`


planet::audio::atomic_linear_gain::atomic_linear_gain(dB_gain const gain) noexcept
: atomic_linear_gain{static_cast<linear_gain>(gain)} {}


void planet::audio::atomic_linear_gain::store(linear_gain const g) {
    multiplier.store(g.multiplier);
}


/// ## `planet::audio::channel`


void planet::audio::save(serialise::save_buffer &sb, channel const &c) {
    sb.save_box(c.box, c.db_g);
}
void planet::audio::load(serialise::box &b, channel &c) {
    if (b.name == dB_gain::box) {
        load(b, c.db_g);
    } else {
        b.named(c.box, c.db_g);
    }
    c.write_through();
}


/// ## `planet::audio::dB_gain`


planet::audio::dB_gain::operator linear_gain() const noexcept {
    if (dB < -127.0f) {
        return linear_gain{};
    } else {
        return linear_gain{std::pow(10.0f, dB / 20.0f)};
    }
}

std::string planet::audio::dB_gain::as_string() const {
    return std::to_string(static_cast<int>(dB + 0.5f)) + "dB";
}


void planet::audio::save(serialise::save_buffer &sb, dB_gain const &g) {
    sb.save_box(g.box, g.dB);
}
void planet::audio::load(serialise::box &b, dB_gain &g) {
    b.named(g.box, g.dB);
}


/// ## `planet::audio::linear_gain`


planet::audio::linear_gain::linear_gain(float const g) : multiplier{g} {}


/// ## `planet::audio::mixer`


auto planet::audio::mixer::output() -> stereo_generator {
    return master.attenuate(raw_mix());
}


auto planet::audio::mixer::raw_mix() -> stereo_generator {
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
    std::optional<planet::audio::stereo_generator> generator;
    while (true) {
        if (clear_flag.load(std::memory_order_relaxed)) {
            clear_flag.store(false, std::memory_order_relaxed);
            playing.store(false, std::memory_order_relaxed);
            generator = {};
            std::scoped_lock _{mtx};
            queue.clear();
        }
        if (not generator) {
            std::scoped_lock _{mtx};
            if (queue.size()) {
                generator = master.attenuate(queue.front()());
                playing.store(true, std::memory_order_relaxed);
            }
        }
        if (generator) {
            while (auto block = generator->next()) {
                if (clear_flag.load(std::memory_order_relaxed)) {
                    /// TODO Micro fade block
                    // co_yield block;
                    break;
                } else {
                    co_yield *block;
                }
            }
            playing.store(false, std::memory_order_relaxed);
            generator = {};
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
    planet::log::debug("Clearing music queue");
    clear_flag.store(true, std::memory_order_relaxed);
    while (clear_flag.load(std::memory_order_relaxed)) {
        co_await ward.sleep(50ms);
    }
}


void planet::audio::music::enqueue(start_tune_function tn) {
    std::scoped_lock _{mtx};
    queue.push_back({std::move(tn)});
}


/// ## `planet::audio::oscillator`


felspar::coro::generator<std::span<float>> silence() {
    std::array<float, planet::audio::default_buffer_samples> buffer{};
    while (true) { co_yield buffer; }
}


felspar::coro::generator<std::span<float>>
        planet::audio::oscillator(float const turns) {
    std::array<float, planet::audio::default_buffer_samples> buffer;
    std::complex const rotate{std::cos(turns * tau), std::sin(turns * tau)};
    std::complex phase{1.0f, 0.0f};
    while (true) {
        for (auto &s : buffer) {
            s = phase.imag();
            phase *= rotate;
        }
        co_yield buffer;
    }
}
