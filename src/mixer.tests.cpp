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


    /// An infinite source that stalls `per_block` before every block, so the
    /// producer thread fills the ring at a controllable, observable rate.
    planet::audio::stereo_generator slow_source(
            float const v, std::chrono::milliseconds const per_block) {
        while (true) {
            std::this_thread::sleep_for(per_block);
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


    /// The producer thread renders into the ring and `next_frame` delivers the
    /// pre-rolled blocks intact (proving the producer/ring/deep-copy path),
    /// with no underruns while the ring is full.
    auto const threaded = felspar::testsuite("mixer.threaded", [](auto check) {
        using namespace std::chrono_literals;
        planet::audio::channel master{planet::audio::dB_gain{0}};
        planet::audio::mixer m{master, 20ms};
        m.add_track(constant_forever(0.25f));
        m.begin();

        /// Let the producer thread fill the whole ring.
        std::this_thread::sleep_for(20ms);

        check(m.activate()) == true;

        std::size_t const frames =
                m.buffer_depth() * planet::audio::default_buffer_samples;
        bool all_constant = true;
        std::size_t produced{};
        for (std::size_t i{}; i < frames; ++i) {
            auto const f = m.next_frame();
            if (f[0] != 0.25f or f[1] != 0.25f) { all_constant = false; }
            ++produced;
        }
        check(all_constant) == true;
        check(produced) == frames;
        check(m.underrun_count()) == std::uint64_t{};
        /// The mixer destructor must stop and join the producer thread
        /// cleanly here (a deadlock would hang the test).
    });


    /// `activate` gates consumption until the producer has pre-filled the ring
    /// (`depth` blocks), so playback only starts with a full `latency` buffered.
    auto const start_gate =
            felspar::testsuite("mixer.start_gate", [](auto check) {
                using namespace std::chrono_literals;
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master, 20ms};
                /// 40ms/block means the ring cannot be full for a while after
                /// `begin`, so the first `activate` reliably sees it unfilled.
                m.add_track(slow_source(0.5f, 40ms));
                m.begin();

                check(m.activate()) == false;
                std::this_thread::sleep_for(40ms * (m.buffer_depth() + 1));
                check(m.activate()) == true;
            });


    /// The producer is bounded to `depth` blocks ahead: even given far more
    /// time than it needs, it must not buffer past the configured latency.
    auto const lead_bound =
            felspar::testsuite("mixer.lead_bound", [](auto check) {
                using namespace std::chrono_literals;
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::mixer m{master, 20ms};
                m.add_track(constant_forever(0.1f));
                m.begin();

                /// 20ms of latency is two ~10.7ms blocks.
                check(m.buffer_depth()) == std::size_t{2};
                /// An unbounded producer would render hundreds of blocks here;
                /// the ring must cap it at exactly `depth`.
                std::this_thread::sleep_for(50ms);
                check(m.buffered_blocks()) == m.buffer_depth();
                check(m.underrun_count()) == std::uint64_t{};
            });


}
