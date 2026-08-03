#include <planet/serialise.hpp>
#include <planet/time.hpp>

#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("serialise/chrono");


    /// Save a value and load it straight back out of the same bytes as a `T`
    template<typename T, typename V>
    T round_trip(V const &value) {
        planet::serialise::save_buffer sb;
        save(sb, value);
        auto const bytes = sb.complete();
        auto lb = planet::serialise::load_buffer{bytes.cmemory()};
        T loaded;
        load(lb, loaded);
        lb.check_empty_or_throw();
        return loaded;
    }


    /// A duration in units that line up with nothing in `std::chrono`
    using samples = std::chrono::duration<std::int64_t, std::ratio<1, 48'000>>;


    auto const same_units = suite.test("duration/same-units", [](auto check) {
        check(round_trip<std::chrono::milliseconds>(
                      std::chrono::milliseconds{1'500})
                      .count())
                == 1'500;
    });


    auto const finer_units =
            suite.test("duration/into-finer-units", [](auto check) {
                /**
                 * The units the count is in are saved with it, so a duration
                 * loaded into finer units than it was saved in comes back as
                 * the same length of time rather than as the same count.
                 */
                check(round_trip<std::chrono::microseconds>(
                              std::chrono::milliseconds{1'500})
                              .count())
                        == 1'500'000;
            });


    auto const coarser_units =
            suite.test("duration/into-coarser-units", [](auto check) {
                /// What the coarser units cannot hold truncates towards zero
                check(round_trip<std::chrono::seconds>(
                              std::chrono::milliseconds{1'500})
                              .count())
                        == 1;
                check(round_trip<std::chrono::seconds>(
                              std::chrono::milliseconds{-1'500})
                              .count())
                        == -1;
            });


    auto const unrelated_units =
            suite.test("duration/unrelated-units", [](auto check) {
                /// Neither unit need be a multiple of the other
                check(round_trip<std::chrono::milliseconds>(samples{48'000})
                              .count())
                        == 1'000;
                check(round_trip<samples>(std::chrono::milliseconds{1'000})
                              .count())
                        == 48'000;
                check(round_trip<samples>(std::chrono::milliseconds{1}).count())
                        == 48;
            });


    auto const large_counts =
            suite.test("duration/large-counts", [](auto check) {
                /**
                 * A count of nanoseconds since 1970 is most of what an
                 * `std::int64_t` can hold, so the scaling has to reach the
                 * count the loaded units want without running over on the way.
                 */
                auto constexpr ns =
                        std::chrono::nanoseconds{1'700'000'000'123'456'789};
                check(round_trip<std::chrono::microseconds>(ns).count())
                        == 1'700'000'000'123'456;
                check(round_trip<std::chrono::nanoseconds>(
                              std::chrono::microseconds{1'700'000'000'123'456})
                              .count())
                        == 1'700'000'000'123'456'000;
            });


    auto const float_counts =
            suite.test("duration/counted-in-floats", [](auto check) {
                using seconds = std::chrono::duration<double>;
                check(round_trip<seconds>(seconds{1.5}).count()) == 1.5;
            });


    auto const old_duration =
            suite.test("duration/before-units-were-saved", [](auto check) {
                /**
                 * A count saved before the units went into the file alongside
                 * it can only be taken to be in the units it is loaded into.
                 */
                planet::serialise::save_buffer sb;
                sb.save_box("_sc::duration", std::int64_t{1'500});
                auto const bytes = sb.complete();
                auto lb = planet::serialise::load_buffer{bytes.cmemory()};
                std::chrono::milliseconds ms;
                load(lb, ms);
                check(ms.count()) == 1'500;
            });


    auto const time_point_same =
            suite.test("time_point/same-units", [](auto check) {
                auto constexpr when = std::chrono::sys_seconds{
                        std::chrono::seconds{1'700'000'000}};
                check(round_trip<std::chrono::sys_seconds>(when) == when)
                        == true;
            });


    auto const time_point_units =
            suite.test("time_point/into-other-units", [](auto check) {
                /**
                 * The time since the epoch is saved as a duration, so the units
                 * ride along with the count and the instant survives a clock
                 * that counts in something else at the other end — which is
                 * what a `std::chrono::system_clock` does, its units being the
                 * standard library's to choose.
                 */
                auto constexpr when = std::chrono::sys_seconds{
                        std::chrono::seconds{1'700'000'000}};
                check(round_trip<
                              std::chrono::sys_time<std::chrono::milliseconds>>(
                              when)
                              .time_since_epoch()
                              .count())
                        == 1'700'000'000'000;
                check(round_trip<std::chrono::sys_seconds>(
                              std::chrono::sys_time<std::chrono::milliseconds>{
                                      std::chrono::milliseconds{
                                              1'700'000'000'500}})
                      == when)
                        == true;
            });


    auto const old_time_point =
            suite.test("time_point/before-units-were-saved", [](auto check) {
                /**
                 * A time point saved before the units went into the file
                 * carries the count alone, in the units of the clock that saved
                 * it, so it can only be taken to be in the units of the time
                 * point it is loaded into.
                 */
                planet::serialise::save_buffer sb;
                sb.save_box("_sc::time_point", std::int64_t{1'700'000'000});
                auto const bytes = sb.complete();
                auto lb = planet::serialise::load_buffer{bytes.cmemory()};
                std::chrono::sys_seconds when;
                load(lb, when);
                check(when.time_since_epoch().count()) == 1'700'000'000;
            });


    auto const bare_steady_count =
            suite.test("steady/reading-back-bare-counts", [](auto check) {
                /**
                 * A file carrying its own reading for the rest of it to be
                 * measured against can still be read, by name. That is what a
                 * log file written before the log kept its times as offsets is,
                 * and reading the counts back as they were written is what
                 * makes the distances between them come out the lengths of time
                 * they were. Only the reading survives -- nothing writes such a
                 * file any more, so the bytes here are spelled out.
                 */
                planet::serialise::save_buffer sb;
                std::chrono::steady_clock::time_point const base{
                        std::chrono::seconds{1'000}};
                sb.save_box(2, "_sc::time_point", base.time_since_epoch());
                sb.save_box(
                        2, "_sc::time_point",
                        (base + std::chrono::milliseconds{250})
                                .time_since_epoch());
                auto const bytes = sb.complete();
                auto lb = planet::serialise::load_buffer{bytes.cmemory()};
                std::chrono::steady_clock::time_point loaded_base, logged;
                auto base_box = planet::serialise::expect_box(lb);
                planet::serialise::detail::load_since_epoch(
                        base_box, loaded_base);
                auto logged_box = planet::serialise::expect_box(lb);
                planet::serialise::detail::load_since_epoch(logged_box, logged);
                check(loaded_base == base) == true;
                check(logged - loaded_base == std::chrono::milliseconds{250})
                        == true;
            });


    auto const game_time = suite.test("game-clock/round-trip", [](auto check) {
        /**
         * Play time is counted from the start of the game, which stays where it
         * is however long the save sits unopened, so the game time comes back
         * the game time it went in as.
         */
        planet::time::clock clock;
        clock.advance_by(std::chrono::seconds{90});
        check(round_trip<planet::time::clock::time_point>(clock.now())
              == clock.now())
                == true;
    });


    auto const old_game_time =
            suite.test("game-clock/before-units-were-saved", [](auto check) {
                planet::serialise::save_buffer sb;
                sb.save_box("_sc::time_point", std::uint64_t{90'000'000'000});
                auto const bytes = sb.complete();
                auto lb = planet::serialise::load_buffer{bytes.cmemory()};
                planet::time::clock::time_point when;
                load(lb, when);
                check(when.time_since_epoch() == std::chrono::seconds{90})
                        == true;
            });


}
