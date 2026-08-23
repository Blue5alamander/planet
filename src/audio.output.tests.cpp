#include <planet/audio/backend.hpp>
#include <planet/audio/channel.hpp>
#include <planet/audio/mixer.hpp>
#include <planet/audio/output.hpp>
#include <planet/log.hpp>
#include <planet/serialise.hpp>
#include <planet/telemetry/counter.hpp>

#include <felspar/test.hpp>

#include <felspar/exceptions/logic_error.hpp>


using namespace std::literals;


namespace {


    /**
     * A minimal `planet::audio::backend`: just enough to prove the interface
     * can be implemented, deleted through a base pointer, and read through a
     * base reference. It records its destruction so the test can confirm the
     * derived destructor actually ran.
     */
    struct stub_backend final : planet::audio::backend {
        bool &destroyed;
        std::atomic<planet::audio::sample_clock> clock{
                planet::audio::sample_clock{512}};

        explicit stub_backend(bool &d) noexcept : destroyed{d} {}
        ~stub_backend() override { destroyed = true; }

        std::atomic<planet::audio::sample_clock> const &
                playback_clock() const noexcept override {
            return clock;
        }
    };


    auto const suite = felspar::testsuite("output", []() {
        planet::log::active = planet::log::level::error;
    });


    /// An infinite source of a constant stereo value.
    planet::audio::stereo_generator constant_forever(float const v) {
        while (true) {
            auto buf = felspar::memory::shared_buffer<float>::allocate(
                    planet::audio::buffer_samples()
                            * planet::audio::stereo_buffer::channels,
                    v);
            co_yield planet::audio::stereo_buffer{std::move(buf)};
        }
    }


    /// Wait, up to a deadline, for the producer to fill the ring.
    bool wait_for_full_ring(planet::audio::mixer &m) {
        auto const deadline = std::chrono::steady_clock::now() + 5s;
        while (m.buffered_blocks() < m.buffer_depth()
               and std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(1ms);
        }
        return m.buffered_blocks() == m.buffer_depth();
    }


    /// Cumulative value of a named counter, read from the registry snapshot.
    std::int64_t counter_value(std::string_view const want) {
        planet::serialise::save_buffer sb;
        planet::telemetry::performance::current_values(sb);
        auto const bytes = sb.complete();
        planet::serialise::load_buffer lb{bytes};
        auto map = planet::telemetry::performance::saved_measurements(lb);
        std::int64_t count{};
        planet::telemetry::load_performance_measurement(
                map, std::string{want}, planet::telemetry::counter::box, count);
        return count;
    }


    /**
     * A backend is usable through the base interface: readable through a
     * `backend &` and deleted through a `backend *`, which the virtual
     * destructor turns into the derived destructor.
     */
    auto const backend = suite.test("backend", [](auto check) {
        bool destroyed = false;
        {
            std::unique_ptr<planet::audio::backend> b =
                    std::make_unique<stub_backend>(destroyed);
            planet::audio::backend &ref = *b;
            check(ref.playback_clock().load())
                    == planet::audio::sample_clock{512};
        }
        check(destroyed) == true;
    });


    auto const rebuild_driver = suite.test(
            "rebuild_driver",
            /**
             * `rebuild_driver` seeds the playback head to the end-time of the
             * first block, and a second call with a different block size
             * re-seeds it.
             */
            [](auto check) {
                auto const saved = planet::audio::buffer_duration();
                planet::audio::output core{[] { return 1.0f; }};

                core.rebuild_driver(512, 2);
                check(core.playback_clock().load())
                        == planet::audio::sample_clock{512};

                core.rebuild_driver(1024, 2);
                check(core.playback_clock().load())
                        == planet::audio::sample_clock{1024};

                planet::audio::active_buffer_duration.store(saved);
            },
            /**
             * `rebuild_driver` publishes the block size into
             * `active_buffer_duration`, so `buffer_samples()` reports the
             * negotiated working block; a second call with a different size
             * re-publishes it.
             */
            [](auto check) {
                auto const saved = planet::audio::buffer_duration();
                planet::audio::output core{[] { return 1.0f; }};

                core.rebuild_driver(1024, 2);
                check(planet::audio::buffer_samples()) == std::size_t{1024};

                core.rebuild_driver(256, 2);
                check(planet::audio::buffer_samples()) == std::size_t{256};

                planet::audio::active_buffer_duration.store(saved);
            },
            /**
             * A mixer attached before a rebuild is re-bound to the fresh
             * driver: a second `rebuild_driver` with a different block count
             * makes the already-attached mixer report the new ring depth and
             * point at the re-seeded playback head. Without the rebind loop it
             * would keep rendering against the previous driver's geometry.
             */
            [](auto check) {
                auto const saved = planet::audio::buffer_duration();
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::output core{[] { return 1.0f; }};
                core.rebuild_driver(512, 2);
                core.reserve_mixers(1);

                planet::audio::mixer m{master};
                core.attach(m);
                check(m.buffer_depth()) == std::size_t{2};

                core.rebuild_driver(1024, 3);
                check(m.buffer_depth()) == std::size_t{3};
                check(m.playback_clock()) == &core.playback_clock();
                check(core.playback_clock().load())
                        == planet::audio::sample_clock{1024};

                planet::audio::active_buffer_duration.store(saved);
            });


    auto const attach = suite.test(
            "attach",
            /**
             * Attaching more mixers than `reserve_mixers` was told to expect
             * throws, so the vector the callback reads lock-free can never be
             * reallocated out from under it.
             */
            [](auto check) {
                auto const saved = planet::audio::buffer_duration();
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::output core{[] { return 1.0f; }};
                core.rebuild_driver(512, 2);
                core.reserve_mixers(1);

                planet::audio::mixer m1{master};
                core.attach(m1);

                planet::audio::mixer m2{master};
                check([&]() {
                    core.attach(m2);
                }).template throws_type<felspar::stdexcept::logic_error>();

                planet::audio::active_buffer_duration.store(saved);
            },
            /**
             * Attaching a real mixer after `rebuild_driver` binds it to the
             * core's driver — the mixer sees the very same playback-head atomic
             * the core publishes — and leaves the playback head readable.
             */
            [](auto check) {
                auto const saved = planet::audio::buffer_duration();
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::output core{[] { return 1.0f; }};
                core.rebuild_driver(512, 2);
                core.reserve_mixers(1);

                planet::audio::mixer m{master};
                core.attach(m);

                check(m.playback_clock()) == &core.playback_clock();
                check(core.playback_clock().load())
                        == planet::audio::sample_clock{512};

                planet::audio::active_buffer_duration.store(saved);
            });


    /// Scratch space for the interleaved stereo block `render` writes.
    using render_buffer = std::array<
            float,
            planet::audio::max_buffer_samples
                    * planet::audio::stereo_buffer::channels>;


    auto const render = suite.test(
            "render",
            /// Two constant tracks sum to their total times the master gain.
            [](auto check) {
                auto const saved = planet::audio::buffer_duration();
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::output core{[] { return 1.0f; }};
                core.rebuild_driver(512, 2);
                std::size_t const block = 512;
                core.reserve_mixers(2);

                planet::audio::mixer m1{master}, m2{master};
                core.attach(m1);
                core.attach(m2);
                m1.add_track(constant_forever(0.25f));
                m2.add_track(constant_forever(0.5f));

                auto buffer = std::make_unique<render_buffer>();
                float *const out = buffer->data();
                planet::by_index(2, [&] { core.render(out, block); });
                check(wait_for_full_ring(m1)) == true;
                check(wait_for_full_ring(m2)) == true;

                core.render(out, block);
                bool summed = true;
                planet::by_index(block, [&](std::size_t const s) {
                    planet::by_index(
                            planet::audio::stereo_buffer::channels,
                            [&](std::size_t const ch) {
                                if (out[s * planet::audio::stereo_buffer::channels
                                        + ch]
                                    != 0.75f) {
                                    summed = false;
                                }
                            });
                });
                check(summed) == true;

                planet::audio::active_buffer_duration.store(saved);
            },
            /// `render` advances the playback head by exactly the frame count.
            [](auto check) {
                auto const saved = planet::audio::buffer_duration();
                planet::audio::output core{[] { return 1.0f; }};
                core.rebuild_driver(512, 2);

                auto const before = core.playback_clock().load();
                auto buffer = std::make_unique<render_buffer>();
                core.render(buffer->data(), 256);
                check(core.playback_clock().load() - before)
                        == planet::audio::sample_clock{256};

                planet::audio::active_buffer_duration.store(saved);
            });


    auto const counters = suite.test(
            "counters",
            /// A signal above 1.0 clips once per channel per frame.
            [](auto check) {
                auto const saved = planet::audio::buffer_duration();
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::output core{[] { return 1.0f; }};
                core.rebuild_driver(512, 2);
                std::size_t const block = 512;
                core.reserve_mixers(1);

                planet::audio::mixer m{master};
                core.attach(m);
                m.add_track(constant_forever(2.0f));

                auto buffer = std::make_unique<render_buffer>();
                float *const out = buffer->data();
                planet::by_index(2, [&] { core.render(out, block); });
                check(wait_for_full_ring(m)) == true;

                auto const before =
                        counter_value("planet_audio__output__clip_count");
                core.render(out, block);
                auto const after =
                        counter_value("planet_audio__output__clip_count");
                check(after - before)
                        == static_cast<std::int64_t>(
                                block * planet::audio::stereo_buffer::channels);

                planet::audio::active_buffer_duration.store(saved);
            },
            /// Draining faster than the producer refills underruns the mixer,
            /// which `render` folds into the output counter.
            [](auto check) {
                auto const saved = planet::audio::buffer_duration();
                planet::audio::channel master{planet::audio::dB_gain{0}};
                planet::audio::output core{[] { return 1.0f; }};
                core.rebuild_driver(512, 2);
                std::size_t const block = 512;
                core.reserve_mixers(1);

                planet::audio::mixer m{master};
                core.attach(m);
                m.add_track(constant_forever(0.1f));

                auto buffer = std::make_unique<render_buffer>();
                float *const out = buffer->data();
                auto const before =
                        counter_value("planet_audio__output__underrun_count");
                bool underran = false;
                auto const deadline = std::chrono::steady_clock::now() + 5s;
                while (std::chrono::steady_clock::now() < deadline) {
                    core.render(out, block);
                    if (m.underrun_count() > 0) {
                        underran = true;
                        break;
                    }
                }
                check(underran) == true;
                check(counter_value("planet_audio__output__underrun_count"))
                        > before;

                planet::audio::active_buffer_duration.store(saved);
            });


    /// A changed master multiplier ramps across the block, not at the boundary.
    auto const ramp = suite.test("ramp", [](auto check) {
        auto const saved = planet::audio::buffer_duration();
        planet::audio::channel master{planet::audio::dB_gain{0}};
        float gain = 0.5f;
        planet::audio::output core{[&] { return gain; }};
        core.rebuild_driver(512, 2);
        std::size_t const block = 512;
        std::size_t const channels = planet::audio::stereo_buffer::channels;
        core.reserve_mixers(1);

        planet::audio::mixer m{master};
        core.attach(m);
        m.add_track(constant_forever(1.0f));

        auto buffer = std::make_unique<render_buffer>();
        float *const out = buffer->data();
        /// Drain the pre-rolled silence at the steady 0.5 multiplier.
        planet::by_index(2, [&] { core.render(out, block); });
        check(wait_for_full_ring(m)) == true;

        gain = 1.0f;
        core.render(out, block);

        /// The block opens at the old multiplier, not the new one.
        check(out[0]) == 0.5f;
        float const midpoint = out[(block / 2) * channels];
        float const endpoint = out[(block - 1) * channels];
        check(midpoint) > out[0];
        check(endpoint) > midpoint;
        /// The ramp is still short of the target by the block's end.
        check(endpoint) < 1.0f;

        planet::audio::active_buffer_duration.store(saved);
    });


}
