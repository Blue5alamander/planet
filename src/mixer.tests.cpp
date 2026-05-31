#include <planet/audio/channel.hpp>
#include <planet/audio/clocks.hpp>
#include <planet/audio/driver.hpp>
#include <planet/audio/gain.hpp>
#include <planet/audio/mixer.hpp>

#include <felspar/memory/shared_buffer.hpp>
#include <felspar/test.hpp>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>


namespace {


    /// A finite source that yields `buffers` blocks of a constant stereo value.
    planet::audio::stereo_generator
            constant_source(std::size_t const buffers, float const v) {
        for (std::size_t b{}; b < buffers; ++b) {
            auto buf = felspar::memory::shared_buffer<float>::allocate(
                    planet::audio::default_buffer_samples
                            * planet::audio::stereo_buffer::channels,
                    v);
            co_yield planet::audio::stereo_buffer{std::move(buf)};
        }
    }


    /// An infinite source of a constant stereo value.
    planet::audio::stereo_generator constant_forever(float const v) {
        while (true) {
            auto buf = felspar::memory::shared_buffer<float>::allocate(
                    planet::audio::default_buffer_samples
                            * planet::audio::stereo_buffer::channels,
                    v);
            co_yield planet::audio::stereo_buffer{std::move(buf)};
        }
    }


    auto const correctness = felspar::testsuite("mixer", [](auto check) {
        planet::audio::channel master{planet::audio::dB_gain{0}};
        planet::audio::mixer m{master};
        m.add_track(constant_source(1, 0.5f));

        auto out = m.output();
        auto first = out.next();
        check(static_cast<bool>(first)) == true;
        check(first->samples()) == planet::audio::default_buffer_samples;
        // Unity master gain, single track -> output matches the source.
        auto const s = (*first)[0];
        check(s[0]) > 0.49f;
        check(s[0]) < 0.51f;
        check(s[1]) > 0.49f;
        check(s[1]) < 0.51f;
    });


    /// Hammer `add_track` from one thread while another drains `output()`.
    /// Guards the data race on `generators`; run under ThreadSanitizer to
    /// actually detect the race (clean with the lock, dirty without it).
    auto const concurrency =
            felspar::testsuite("mixer.concurrency", [](auto check) {
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master};
                auto out = m.output();

                std::atomic<bool> stop{false};
                std::thread producer{[&]() {
                    for (std::size_t i{}; i < 2'000 and not stop.load(); ++i) {
                        m.add_track(constant_source(3, 0.01f));
                    }
                }};

                std::size_t pulled{};
                for (std::size_t i{}; i < 2'000; ++i) {
                    auto block = out.next();
                    check(static_cast<bool>(block)) == true;
                    check(block->samples())
                            == planet::audio::default_buffer_samples;
                    ++pulled;
                }
                stop.store(true);
                producer.join();

                check(pulled) == 2'000u;
            });


    /// The ring is pre-rolled with silence by `bind_driver`, so the
    /// callback hears `depth` blocks of zeros first; the track added before
    /// `begin` then becomes audible exactly `latency` later — the
    /// fixed-latency promise this design is built on.
    auto const threaded = felspar::testsuite("mixer.threaded", [](auto check) {
        using namespace std::chrono_literals;
        planet::audio::channel master{planet::audio::dB_gain{0}};
        planet::audio::mixer m{master};
        planet::audio::driver drv{planet::audio::default_buffer_samples, 2};
        m.bind_driver(drv);
        m.add_track(constant_forever(0.25f));
        m.begin();

        std::size_t const block = planet::audio::default_buffer_samples;
        std::size_t const depth = m.buffer_depth();

        /// The first `depth` blocks must be the constructor's pre-rolled
        /// silence, regardless of how fast the producer thread is.
        bool silence_clean = true;
        for (std::size_t i{}; i < depth * block; ++i) {
            auto const f = m.next_frame();
            if (f[0] != 0.0f or f[1] != 0.0f) { silence_clean = false; }
        }
        check(silence_clean) == true;

        /// Let the producer fill the ring with track audio before draining
        /// it in a tight loop — otherwise the consumer can outrun the
        /// per-wrap render and surface spurious underruns.
        std::this_thread::sleep_for(20ms);

        bool track_clean = true;
        for (std::size_t i{}; i < depth * block; ++i) {
            auto const f = m.next_frame();
            if (f[0] != 0.25f or f[1] != 0.25f) { track_clean = false; }
        }
        check(track_clean) == true;
        check(m.underrun_count()) == std::uint64_t{};
        /// The mixer destructor must stop and join the producer thread
        /// cleanly here (a deadlock would hang the test).
    });


    /// `bind_driver` makes the driver's playback-head atomic findable
    /// through the mixer; an unbound mixer reports null.
    auto const playback_clock =
            felspar::testsuite("mixer.playback_clock", [](auto check) {
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master};

                check(m.playback_clock()) == nullptr;

                planet::audio::driver drv{
                        planet::audio::default_buffer_samples, 2};
                drv.playback_head.store(planet::audio::sample_clock{512});
                m.bind_driver(drv);

                check(m.playback_clock()) == &drv.playback_head;
                check(m.playback_clock()->load())
                        == planet::audio::sample_clock{512};

                drv.playback_head.store(planet::audio::sample_clock{1024});
                check(m.playback_clock()->load())
                        == planet::audio::sample_clock{1024};
            });


    /// `bind_driver` is re-callable: simulates an `audio_output::reconnect`
    /// where SDL renegotiates the device and the existing mixer has to
    /// pick up a fresh driver. Pre-fix, the second `bind_driver` would
    /// stomp the slot buffers under a running producer thread and the
    /// second `begin()` would terminate the program by reassigning a
    /// joinable `std::thread`. The contract is: stops the producer
    /// cleanly, re-derives the ring depth from the new driver,
    /// re-pre-rolls silence, and `begin()` afterwards restarts without
    /// deadlock.
    auto const rebind =
            felspar::testsuite("mixer.rebind", [](auto check, auto &log) {
                using namespace std::chrono_literals;
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master};
                planet::audio::driver drv1{
                        planet::audio::default_buffer_samples, 2};
                m.bind_driver(drv1);
                m.add_track(constant_forever(0.25f));
                m.begin();

                /// Cycle the first ring fully so the producer thread
                /// actually does some work (its `slots_free.acquire` is
                /// gated on the consumer freeing slots) — otherwise the
                /// rebind teardown path would never observe a running
                /// producer.
                std::size_t const block1 = drv1.block_size;
                std::size_t const depth1 = m.buffer_depth();
                for (std::size_t i{}; i < depth1 * block1; ++i) {
                    (void)m.next_frame();
                }
                std::this_thread::sleep_for(20ms);
                for (std::size_t i{}; i < depth1 * block1; ++i) {
                    (void)m.next_frame();
                }
                log << "after phase 1 underruns=" << m.underrun_count() << "\n";

                /// Different block_count so the rebind has to re-derive
                /// the ring depth from the new driver.
                planet::audio::driver drv2{
                        planet::audio::default_buffer_samples, 3};
                m.bind_driver(drv2);
                check(m.buffer_depth()) == std::size_t{3};

                /// After the rebind the ring is silence-pre-rolled at
                /// the new block size — that is the rebind's whole
                /// purpose. Verify the first block reads as silence.
                std::size_t const block2 = drv2.block_size;
                m.begin();
                bool silence_clean = true;
                for (std::size_t i{}; i < block2; ++i) {
                    auto const f = m.next_frame();
                    if (f[0] != 0.0f or f[1] != 0.0f) {
                        silence_clean = false;
                        log << "non-silence at i=" << i << ": (" << f[0] << ", "
                            << f[1] << ")\n";
                    }
                }
                check(silence_clean) == true;

                /// And the mixer destructor must still join the new
                /// producer thread cleanly here — a deadlock would
                /// hang the test.
            });


    /// Pull `blocks` blocks straight from `output()` (no producer thread) and
    /// return the flattened left-channel samples. Single-threaded, so the
    /// scheduling maths is exercised deterministically with no timing jitter.
    std::vector<float>
            pull_left(planet::audio::mixer &m, std::size_t const blocks) {
        std::vector<float> out;
        auto gen = m.output();
        for (std::size_t b{}; b < blocks; ++b) {
            auto block = gen.next();
            if (not block) { break; }
            for (std::size_t s{}; s < block->samples(); ++s) {
                out.push_back((*block)[s][0]);
            }
        }
        return out;
    }


    /// A track scheduled at the driver's `wall_clock_epoch` has a zero target
    /// position, so it plays from the very first sample — identical to the
    /// immediate overload.
    auto const schedule_immediate =
            felspar::testsuite("mixer.schedule.immediate", [](auto check) {
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master};
                planet::audio::driver drv{
                        planet::audio::default_buffer_samples, 2};
                m.bind_driver(drv);
                m.add_track(constant_forever(0.25f), drv.wall_clock_epoch);

                auto const left = pull_left(m, 2);
                check(left.front()) == 0.25f;
                check(left.back()) == 0.25f;
            });


    /// A track scheduled 20ms after the epoch (== 960 samples at 48kHz) is
    /// preceded by exactly that many samples of silence, then plays.
    auto const schedule_delayed =
            felspar::testsuite("mixer.schedule.delayed", [](auto check) {
                using namespace std::chrono_literals;
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master};
                planet::audio::driver drv{
                        planet::audio::default_buffer_samples, 2};
                m.bind_driver(drv);

                /// 20ms is an exact 960 samples (and an exact 20'000'000ns), so
                /// neither the wall->sample nor sample->wall conversion rounds.
                std::size_t const expected_silence = 960;
                m.add_track(
                        constant_forever(0.25f), drv.wall_clock_epoch + 20ms);

                auto const left = pull_left(m, 4);
                bool silence_clean = true;
                for (std::size_t i{}; i < expected_silence; ++i) {
                    if (left[i] != 0.0f) { silence_clean = false; }
                }
                check(silence_clean) == true;
                bool audio_clean = true;
                for (std::size_t i{expected_silence}; i < left.size(); ++i) {
                    if (left[i] != 0.25f) { audio_clean = false; }
                }
                check(audio_clean) == true;
                check(left[expected_silence - 1]) == 0.0f;
                check(left[expected_silence]) == 0.25f;
            });


    /// A track scheduled in the past relative to the epoch clamps to "as soon
    /// as possible" — it plays from the first sample with no negative delay.
    auto const schedule_late =
            felspar::testsuite("mixer.schedule.late", [](auto check) {
                using namespace std::chrono_literals;
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master};
                planet::audio::driver drv{
                        planet::audio::default_buffer_samples, 2};
                m.bind_driver(drv);
                m.add_track(constant_forever(0.25f), drv.wall_clock_epoch - 1s);

                auto const left = pull_left(m, 2);
                check(left.front()) == 0.25f;
                check(left.back()) == 0.25f;
            });


    /// On a mixer with no driver bound the wall-clock cannot be resolved, so a
    /// scheduled track falls back to playing as soon as possible.
    auto const schedule_unbound =
            felspar::testsuite("mixer.schedule.unbound", [](auto check) {
                using namespace std::chrono_literals;
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master};
                m.add_track(
                        constant_forever(0.25f),
                        std::chrono::steady_clock::now() + 1s);

                auto const left = pull_left(m, 2);
                check(left.front()) == 0.25f;
                check(left.back()) == 0.25f;
            });


    /// The producer is bounded to `depth` blocks ahead: even given far more
    /// time than it needs, it must not buffer past the configured latency.
    auto const lead_bound =
            felspar::testsuite("mixer.lead_bound", [](auto check) {
                using namespace std::chrono_literals;
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master};
                planet::audio::driver drv{
                        planet::audio::default_buffer_samples, 2};
                m.bind_driver(drv);
                m.add_track(constant_forever(0.1f));
                m.begin();

                /// 1024 samples of latency is two 512-sample blocks.
                check(m.buffer_depth()) == std::size_t{2};
                /// An unbounded producer would render hundreds of blocks here;
                /// the ring must cap it at exactly `depth`.
                std::this_thread::sleep_for(50ms);
                check(m.buffered_blocks()) == m.buffer_depth();
                check(m.underrun_count()) == std::uint64_t{};
            });


}
