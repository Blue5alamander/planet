#include <planet/audio/backend.hpp>
#include <planet/audio/channel.hpp>
#include <planet/audio/mixer.hpp>
#include <planet/audio/mixing_core.hpp>

#include <felspar/test.hpp>

#include <felspar/exceptions/logic_error.hpp>

#include <atomic>
#include <memory>


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


    auto const suite = felspar::testsuite("mixing_core");


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
                planet::audio::mixing_core core{[] { return 1.0f; }};

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
                planet::audio::mixing_core core{[] { return 1.0f; }};

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
                planet::audio::mixing_core core{[] { return 1.0f; }};
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
                planet::audio::mixing_core core{[] { return 1.0f; }};
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
                planet::audio::mixing_core core{[] { return 1.0f; }};
                core.rebuild_driver(512, 2);
                core.reserve_mixers(1);

                planet::audio::mixer m{master};
                core.attach(m);

                check(m.playback_clock()) == &core.playback_clock();
                check(core.playback_clock().load())
                        == planet::audio::sample_clock{512};

                planet::audio::active_buffer_duration.store(saved);
            });


}
