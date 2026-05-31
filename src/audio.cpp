#include <planet/audio/channel.hpp>
#include <planet/audio/driver.hpp>
#include <planet/audio/gain.hpp>
#include <planet/audio/mixer.hpp>
#include <planet/audio/music.hpp>
#include <planet/audio/oscillator.hpp>
#include <planet/functional.hpp>
#include <planet/log.hpp>
#include <planet/serialise.hpp>
#include <planet/numbers.hpp>

#include <felspar/memory/accumulation_buffer.hpp>

#include <algorithm>
#include <cmath>
#include <exception>


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


planet::audio::dB_gain planet::audio::dB_gain::from_linear_gain(
        float const multiplier) noexcept {
    if (multiplier <= 0.0f) { return dB_gain{-128.0f}; }
    return dB_gain{20.0f * std::log10(multiplier)};
}

planet::audio::dB_gain::dB_gain(linear_gain const lg) noexcept
: dB_gain{from_linear_gain(lg.load())} {}
planet::audio::dB_gain::dB_gain(atomic_linear_gain const &ag) noexcept
: dB_gain{from_linear_gain(ag.load())} {}


planet::audio::dB_gain::operator linear_gain() const noexcept {
    return linear_gain{static_cast<float>(*this)};
}
planet::audio::dB_gain::operator float() const noexcept {
    if (dB < -127.0f) {
        return {};
    } else {
        return std::pow(10.0f, dB / 20.0f);
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


namespace {
    auto const dB_gain_print = planet::log::format(
            planet::audio::dB_gain::box,
            [](std::ostream &os, planet::serialise::box &box) {
                planet::audio::dB_gain g;
                load(box, g);
                os << g.as_string();
            });
}


/// ## `planet::audio::driver`


planet::audio::driver::driver(
        std::size_t const block_size, std::size_t const block_count) noexcept
: block_size{block_size},
  block_count{block_count},
  latency{sample_clock{
          static_cast<sample_clock::rep>(block_size * block_count)}} {
    if (block_count > mixer::max_ring_depth) {
        planet::log::critical(
                "Audio driver block_count", block_count,
                "exceeds the mixer's maximum ring depth of",
                mixer::max_ring_depth);
    }
    if (block_count == 0) {
        planet::log::critical("Audio driver block_count must be at least 1");
    }
}


/// ## `planet::audio::mixer`


planet::audio::mixer::mixer(channel &c) : master{c}, slots_free{0} {
    incoming.reserve(generators.capacity());
}


void planet::audio::mixer::bind_driver(driver const &d) noexcept {
    /**
     * Safe to call again on an already-running mixer: the audio device must
     * already have been quiesced by the caller (so the callback thread is not
     * consuming `next_frame`), then we stop the producer, drain leftover
     * `slots_free` permits, and reset every piece of ring state before
     * re-allocating slots for the new driver's `block_size`. The caller
     * invokes `begin()` afterwards.
     */
    if (producer.joinable()) {
        stop_flag.store(true, std::memory_order_release);
        slots_free.release();
        producer.join();
        stop_flag.store(false, std::memory_order_release);
        while (slots_free.try_acquire()) {}
        read_slot = 0;
        read_marker = 0;
        write_slot = 0;
    }

    drv = &d;
    /**
     * Declare the ring pre-rolled with silence. Each slot is given a freshly
     * allocated zero-filled `shared_buffer<float>` of one block, so the first
     * `drv->block_count` blocks the callback consumes are silence — exactly
     * what synchronous pre-roll would have produced before any track was
     * added. The producer is held off (`slots_free` starts at zero) until the
     * callback frees its first slot, so it can never race the callback into a
     * slot the callback is still reading.
     */
    for (auto &s : slots) {
        s = felspar::memory::shared_buffer<float>::allocate(
                drv->block_size * stereo_buffer::channels, 0.0f);
    }
    ready_count.store(
            static_cast<int>(drv->block_count), std::memory_order_release);
}


auto planet::audio::mixer::output() -> stereo_generator {
    return master.attenuate(raw_mix());
}


auto planet::audio::mixer::raw_mix() -> stereo_generator {
    felspar::memory::accumulation_buffer<float> output{
            default_buffer_samples * 50};
    while (true) {
        for (auto &waiting : incoming.consume()) {
            if (generators.has_room()) {
                generators.push_back({std::move(waiting)});
            }
        }
        generators.erase_if([&output](auto &gen) {
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


planet::audio::mixer::~mixer() {
    if (producer.joinable()) {
        stop_flag.store(true, std::memory_order_release);
        slots_free.release();
        producer.join();
    }
}


void planet::audio::mixer::begin() {
    if (drv == nullptr) {
        planet::log::critical(
                "mixer::begin called before bind_driver — no driver bound, "
                "so the producer would block forever on slots_free");
    }
    producer = std::thread{[this]() { run(); }};
}


void planet::audio::mixer::run() noexcept {
    try {
        auto gen = output();
        /**
         * The producer's own accumulation_buffer. Each iteration copies the
         * upstream block's samples into the accumulator and yields the head
         * `default_buffer_samples * channels` floats as a
         * `shared_buffer<float>` via `first()` — the same idiom `raw_mix` and
         * `gain` already use. Each call returns a fresh refcounted slice; the
         * accumulator grows and reallocates as needed, keeping older slices
         * alive via the shared control block until the last consumer (including
         * any future tap subscribers) drops them.
         */
        felspar::memory::accumulation_buffer<float> publish{
                default_buffer_samples * stereo_buffer::channels * 50};
        while (not stop_flag.load(std::memory_order_acquire)) {
            slots_free.acquire();
            if (stop_flag.load(std::memory_order_acquire)) { break; }
            auto block = gen.next();
            std::size_t const block_floats =
                    drv->block_size * stereo_buffer::channels;
            publish.ensure_length(block_floats);
            if (block) {
                std::size_t const n = std::min<std::size_t>(
                        block->samples(), drv->block_size);
                planet::by_index(n, [&](std::size_t const s) {
                    planet::by_index(
                            stereo_buffer::channels, [&](std::size_t const ch) {
                                publish[s * stereo_buffer::channels + ch] =
                                        (*block)[s][ch];
                            });
                });
                planet::by_index(
                        block_floats - n * stereo_buffer::channels,
                        [&](std::size_t const i) {
                            publish[n * stereo_buffer::channels + i] = 0.0f;
                        });
            } else {
                planet::by_index(block_floats, [&](std::size_t const i) {
                    publish[i] = 0.0f;
                });
            }
            /**
             * Publish a fresh refcounted slice into the slot. The previous
             * slot's slice is dropped here on the producer thread; if no
             * other consumer still holds it, its refcount falls back into
             * the accumulator's vector and the memory is reclaimed off the
             * real-time audio thread.
             */
            slots[write_slot] = publish.first(block_floats);
            write_slot = (write_slot + 1) % drv->block_count;
            ready_count.fetch_add(1, std::memory_order_release);
        }
    } catch (std::exception const &e) {
        planet::log::critical(
                "The audio mixer producer thread caught an exception while "
                "rendering; an audio generator must not throw",
                e.what());
    } catch (...) {
        planet::log::critical(
                "The audio mixer producer thread caught a non-standard "
                "exception while rendering; an audio generator must not throw");
    }
}


/// ## `planet::audio::music`


planet::audio::stereo_generator planet::audio::music::output() {
    felspar::memory::accumulation_buffer<float> output{
            default_buffer_samples * 50};
    std::optional<planet::audio::stereo_generator> generator;
    while (true) {
        if (clear_flag.load(std::memory_order_relaxed)) {
            playing.store(false, std::memory_order_relaxed);
            generator = {};
            std::scoped_lock _{mtx};
            queue.clear();
        }
        if (not generator) {
            std::scoped_lock _{mtx};
            if (queue.size()) {
                auto next = queue.front().next();
                if (next) {
                    generator = std::move(next.value());
                } else {
                    queue.erase(queue.begin());
                }
            }
        }
        if (generator) {
            playing.store(true, std::memory_order_relaxed);
            while (auto block = generator->next()) {
                if (clear_flag.load(std::memory_order_relaxed)) {
                    co_yield micro_fade_out(*block, output);
                    break;
                } else {
                    co_yield *block;
                }
            }
            playing.store(false, std::memory_order_relaxed);
            generator = {};
        } else {
            playing.store(false, std::memory_order_relaxed);
            output.ensure_length(
                    default_buffer_samples * stereo_buffer::channels);
            co_yield output.first(
                    default_buffer_samples * stereo_buffer::channels);
        }
    }
}


felspar::coro::task<void>
        planet::audio::music::clear(felspar::io::warden &ward) {
    planet::log::info("Clearing music queue");
    clear_flag.store(true, std::memory_order_relaxed);
    while (playing.load(std::memory_order_relaxed)) {
        co_await ward.sleep(50ms);
        planet::log::debug("Still waiting for music to clear");
    }
    clear_flag.store(false, std::memory_order_relaxed);
    planet::log::debug("Music cleared");
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
