#include <planet/audio/channel.hpp>
#include <planet/audio/clocks.hpp>
#include <planet/audio/gain.hpp>
#include <planet/audio/mixer.hpp>

#include <felspar/memory/shared_buffer.hpp>
#include <felspar/test.hpp>

#include <atomic>
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


}
