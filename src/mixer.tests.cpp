#include <planet/audio/channel.hpp>
#include <planet/audio/clocks.hpp>
#include <planet/audio/gain.hpp>
#include <planet/audio/mixer.hpp>

#include <felspar/memory/shared_buffer.hpp>
#include <felspar/test.hpp>

#include <atomic>
#include <chrono>
#include <thread>


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


    /// The ring is pre-rolled with silence by `bind_playback_clock`, so the
    /// callback hears `depth` blocks of zeros first; the track added before
    /// `begin` then becomes audible exactly `latency` later — the
    /// fixed-latency promise this design is built on.
    auto const threaded = felspar::testsuite("mixer.threaded", [](auto check) {
        using namespace std::chrono_literals;
        planet::audio::channel master{planet::audio::dB_gain{0}};
        planet::audio::mixer m{master};
        std::atomic<planet::audio::sample_clock> head{};
        m.bind_playback_clock(head, planet::audio::sample_clock{1024});
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


    /// `bind_playback_clock` makes an externally-owned atomic findable
    /// through the mixer; an unbound mixer reports null.
    auto const playback_clock =
            felspar::testsuite("mixer.playback_clock", [](auto check) {
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master};

                check(m.playback_clock()) == nullptr;

                std::atomic<planet::audio::sample_clock> head{
                        planet::audio::sample_clock{512}};
                m.bind_playback_clock(head, planet::audio::sample_clock{1024});

                check(m.playback_clock()) == &head;
                check(m.playback_clock()->load())
                        == planet::audio::sample_clock{512};

                head.store(planet::audio::sample_clock{1024});
                check(m.playback_clock()->load())
                        == planet::audio::sample_clock{1024};
            });


    /// The producer is bounded to `depth` blocks ahead: even given far more
    /// time than it needs, it must not buffer past the configured latency.
    auto const lead_bound =
            felspar::testsuite("mixer.lead_bound", [](auto check) {
                using namespace std::chrono_literals;
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master};
                std::atomic<planet::audio::sample_clock> head{};
                m.bind_playback_clock(head, planet::audio::sample_clock{1024});
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
