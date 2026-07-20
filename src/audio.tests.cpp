#include <planet/audio/clocks.hpp>
#include <planet/audio/gain.hpp>
#include <planet/audio/oscillator.hpp>
#include <planet/serialise.hpp>
#include <felspar/test.hpp>


static_assert(
        planet::audio::max_buffer_duration
                >= planet::audio::default_buffer_duration,
        "The cap must be at least as large as the default buffer so arrays sized by it always have room for the working block");
static_assert(
        planet::audio::max_buffer_samples
                == planet::audio::max_buffer_duration.count(),
        "max_buffer_samples must track max_buffer_duration");


namespace {


    auto const suite = felspar::testsuite("audio", [](auto check) {
        auto const gain = planet::audio::dB_gain{-3};
        planet::audio::linear_gain const linear{gain};
        // Expect ~0.707f
        check(linear.load()) > 0.707f;
        check(linear.load()) < 0.708f;

        // Round-trip: linear -> dB should recover the original value
        planet::audio::dB_gain const back{linear};
        check(back.dB) > -3.01f;
        check(back.dB) < -2.99f;

        // Zero/negative linear gain -> silent floor
        planet::audio::linear_gain const zero{0.0f};
        check(planet::audio::dB_gain{zero}.dB) == -128.0f;
    });


    auto const serialisation = suite.test("serialise", [](auto check) {
        planet::serialise::save_buffer sb;
        save(sb, planet::audio::linear_gain{0.5f});
        auto const bytes = sb.complete();
        auto lb = planet::serialise::load_buffer{bytes.cmemory()};
        planet::audio::linear_gain loaded;
        load(lb, loaded);
        lb.check_empty_or_throw();
        check(loaded.load()) == 0.5f;
    });


    /**
     * The published working block starts at the compile-time seed, and a store
     * is visible to both accessors. The store is restored afterwards so the
     * suite stays order-independent — every other test still sees the 512
     * default.
     *
     * If this test ever fails then follow on tests will have the wrong value.
     */
    auto const buffer_size =
            felspar::testsuite("audio.buffer_size", [](auto check) {
                check(planet::audio::buffer_samples()) == std::size_t{512};
                check(planet::audio::buffer_duration())
                        == planet::audio::sample_clock{512};

                auto const saved = planet::audio::buffer_duration();
                planet::audio::active_buffer_duration.store(
                        planet::audio::sample_clock{1024});
                check(planet::audio::buffer_samples()) == std::size_t{1024};
                check(planet::audio::buffer_duration())
                        == planet::audio::sample_clock{1024};
                planet::audio::active_buffer_duration.store(saved);
            });


    /**
     * `silence` and `oscillator` are the engine's sources: they size their
     * storage by the cap and yield only the first `buffer_samples()` entries,
     * so every pull returns exactly the working block. Raising the working size
     * to 1024 makes both yield 1024, and the previous value is restored so the
     * suite stays order-independent.
     */
    auto const generator_block_size =
            felspar::testsuite("audio.generator_block_size", [](auto check) {
                auto const yields_working_block =
                        [&check](auto generator, std::size_t const expected) {
                            auto block = generator.next();
                            check(static_cast<bool>(block)) == true;
                            check(block->size()) == expected;
                        };
                yields_working_block(planet::audio::silence(), 512);
                yields_working_block(planet::audio::oscillator(0.01f), 512);
                check(planet::audio::buffer_samples()) == std::size_t{512};

                auto const saved = planet::audio::buffer_duration();
                planet::audio::active_buffer_duration.store(
                        planet::audio::sample_clock{1024});
                yields_working_block(planet::audio::silence(), 1024);
                yields_working_block(planet::audio::oscillator(0.01f), 1024);
                planet::audio::active_buffer_duration.store(saved);
            });


}
