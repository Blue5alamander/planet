#include <planet/audio/channel.hpp>
#include <planet/audio/clocks.hpp>
#include <planet/audio/driver.hpp>
#include <planet/audio/gain.hpp>
#include <planet/audio/mixer.hpp>
#include <planet/audio/tap.hpp>

#include <felspar/memory/shared_buffer.hpp>
#include <felspar/test.hpp>

#include <array>
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
        /// The immediate overload is ASAP by design, so it is not counted.
        check(m.asap_scheduled_count()) == 0;
    });


    /**
     * Hammer `add_track` from one thread while another drains `output()`.
     * Guards the data race on `generators`; run under ThreadSanitizer to
     * actually detect the race (clean with the lock, dirty without it).
     */
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


    /**
     * The ring is pre-rolled with silence by `bind_driver`, so the callback
     * hears `depth` blocks of zeros first; the track added before `begin` then
     * becomes audible exactly `latency` later — the fixed-latency promise this
     * design is built on.
     */
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

        /**
         * The first `depth` blocks must be the constructor's pre-rolled
         * silence, regardless of how fast the producer thread is.
         */
        bool silence_clean = true;
        for (std::size_t i{}; i < depth * block; ++i) {
            auto const f = m.next_frame();
            if (f[0] != 0.0f or f[1] != 0.0f) { silence_clean = false; }
        }
        check(silence_clean) == true;

        /**
         * Let the producer fill the ring with track audio before draining it in
         * a tight loop — otherwise the consumer can outrun the per-wrap render
         * and surface spurious underruns.
         */
        std::this_thread::sleep_for(20ms);

        bool track_clean = true;
        for (std::size_t i{}; i < depth * block; ++i) {
            auto const f = m.next_frame();
            if (f[0] != 0.25f or f[1] != 0.25f) { track_clean = false; }
        }
        check(track_clean) == true;
        check(m.underrun_count()) == std::uint64_t{};
        /**
         * The mixer destructor must stop and join the producer thread cleanly
         * here (a deadlock would hang the test).
         */
    });


    /**
     * A `tap` passed to the mixer receives a copy of every block the producer
     * thread publishes. With a single constant track at unity master gain, each
     * forwarded block is a full, refcounted slice of the track's value — the
     * same stream the audio callback consumes.
     */
    auto const tapping = felspar::testsuite("mixer.tap", [](auto check) {
        using namespace std::chrono_literals;
        planet::audio::channel master{planet::audio::dB_gain{0}};
        /// Declared before the mixer so it outlives the producer thread the
        /// mixer destructor joins.
        planet::audio::tap recorder;
        recorder.reserve(planet::audio::mixer::max_ring_depth * 4);
        std::array<planet::audio::tap *, 1> taps{&recorder};
        planet::audio::mixer m{master, taps};
        planet::audio::driver drv{planet::audio::default_buffer_samples, 2};
        m.bind_driver(drv);
        m.add_track(constant_forever(0.25f));
        m.begin();

        std::size_t const block = planet::audio::default_buffer_samples;
        std::size_t const depth = m.buffer_depth();

        /**
         * Free slots by consuming, letting the bounded producer publish several
         * blocks; the sleeps give it time to render between drains.
         */
        std::this_thread::sleep_for(20ms);
        for (std::size_t i{}; i < depth * block * 3; ++i) {
            (void)m.next_frame();
        }
        std::this_thread::sleep_for(20ms);

        std::size_t received{};
        bool clean = true;
        for (auto &b : recorder.consume()) {
            ++received;
            if (b.size() != block * planet::audio::stereo_buffer::channels) {
                clean = false;
            }
            for (auto const s : b) {
                if (s != 0.25f) { clean = false; }
            }
        }
        check(received) > std::size_t{};
        check(clean) == true;
        /**
         * The mixer destructor must stop and join the producer thread here
         * before `recorder` is destroyed (a deadlock would hang the test).
         */
    });


    /**
     * `bind_driver` makes the driver's playback-head atomic findable through
     * the mixer; an unbound mixer reports null.
     */
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


    /**
     * `bind_driver` is re-callable: simulates an `audio_output::reconnect`
     * where SDL renegotiates the device and the existing mixer has to pick up a
     * fresh driver. Pre-fix, the second `bind_driver` would stomp the slot
     * buffers under a running producer thread and the second `begin()` would
     * terminate the program by reassigning a joinable `std::thread`. The
     * contract is: stops the producer cleanly, re-derives the ring depth from
     * the new driver, re-pre-rolls silence, and `begin()` afterwards restarts
     * without deadlock.
     */
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

                /**
                 * Cycle the first ring fully so the producer thread actually
                 * does some work (its `slots_free.acquire` is gated on the
                 * consumer freeing slots) — otherwise the rebind teardown path
                 * would never observe a running producer.
                 */
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

                /**
                 * Different block_count so the rebind has to re-derive the ring
                 * depth from the new driver.
                 */
                planet::audio::driver drv2{
                        planet::audio::default_buffer_samples, 3};
                m.bind_driver(drv2);
                check(m.buffer_depth()) == std::size_t{3};

                /**
                 * After the rebind the ring is silence-pre-rolled at the new
                 * block size — that is the rebind's whole purpose. Verify the
                 * first block reads as silence.
                 */
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

                /**
                 * And the mixer destructor must still join the new producer
                 * thread cleanly here — a deadlock would hang the test.
                 */
            });


    /**
     * Pull whole blocks straight from `output()` (no producer thread) until at
     * least `least` of audio (measured on the `sample_clock`) has been
     * collected, then return the flattened left channel. Block-aligned, so the
     * result may run a little past `least`.
     */
    std::vector<float> pull_left(
            planet::audio::mixer &m, planet::audio::sample_clock const least) {
        std::vector<float> out;
        auto gen = m.output();
        while (out.size() < static_cast<std::size_t>(least.count())) {
            auto block = gen.next();
            if (not block) { break; }
            for (std::size_t s{}; s < block->samples(); ++s) {
                out.push_back((*block)[s][0]);
            }
        }
        return out;
    }


    /**
     * A track scheduled at the driver's `wall_clock_epoch` is pinned a fixed
     * `driver::latency` ahead of the producer's write head, so it is preceded
     * by exactly `latency` samples of silence and then plays.
     */
    auto const schedule_immediate =
            felspar::testsuite("mixer.schedule.immediate", [](auto check) {
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master};
                planet::audio::driver drv{
                        planet::audio::default_buffer_samples, 2};
                m.bind_driver(drv);
                m.add_track(constant_forever(0.25f), drv.wall_clock_epoch);

                auto const expected_silence =
                        static_cast<std::size_t>(drv.latency.count());
                /**
                 * Cover the silent lead-in (the latency headroom) plus a block
                 * of audio to verify.
                 */
                auto const left = pull_left(
                        m,
                        drv.latency + planet::audio::default_buffer_duration);
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
                /// On time (ahead of the write head), so not counted as ASAP.
                check(m.asap_scheduled_count()) == 0;
            });


    /**
     * A track scheduled 20ms after the epoch (== 960 samples at 48kHz) is
     * preceded by that many samples of silence plus the fixed `driver::latency`
     * headroom, then plays.
     */
    auto const schedule_delayed =
            felspar::testsuite("mixer.schedule.delayed", [](auto check) {
                using namespace std::chrono_literals;
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master};
                planet::audio::driver drv{
                        planet::audio::default_buffer_samples, 2};
                m.bind_driver(drv);

                /**
                 * 20ms is an exact 960 samples (and an exact 20'000'000ns), so
                 * neither the wall->sample nor sample->wall conversion rounds.
                 * The scheduled position is pinned `driver::latency` ahead, so
                 * that constant is added to the leading silence.
                 */
                std::size_t const expected_silence =
                        960 + static_cast<std::size_t>(drv.latency.count());
                m.add_track(
                        constant_forever(0.25f), drv.wall_clock_epoch + 20ms);

                /**
                 * Cover the silent lead-in (the 20ms delay plus the latency
                 * headroom) plus a block of audio to verify.
                 */
                auto const left = pull_left(
                        m,
                        20ms + drv.latency
                                + planet::audio::default_buffer_duration);
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
                /// On time (ahead of the write head), so not counted as ASAP.
                check(m.asap_scheduled_count()) == 0;
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

                auto const left =
                        pull_left(m, planet::audio::default_buffer_duration);
                check(left.front()) == 0.25f;
                check(left.back()) == 0.25f;
                /// Its target slipped behind the write head: counted as ASAP.
                check(m.asap_scheduled_count()) == 1;
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

                auto const left =
                        pull_left(m, planet::audio::default_buffer_duration);
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
