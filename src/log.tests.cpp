#include <planet/log.hpp>
#include <planet/map/hex.hpp>
#include <planet/serialise.hpp>
#include <planet/time.hpp>

#include <felspar/test.hpp>

#include <chrono>
#include <optional>
#include <sstream>


using namespace std::chrono_literals;


namespace {


    auto const suite = felspar::testsuite("log");


    auto const hex_formatter = suite.test(
            "planet::map::hex::coordinates",
            [](auto check) {
                planet::map::hex::coordinates const pos{3, 5};

                planet::serialise::save_buffer sb;
                planet::map::hex::save(sb, pos);
                auto const bytes = sb.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "hex@(3, 5)";
            },
            [](auto check) {
                planet::map::hex::coordinates const pos{-2, -4};

                planet::serialise::save_buffer sb;
                planet::map::hex::save(sb, pos);
                auto const bytes = sb.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "hex@(-2, -4)";
            });


    auto const square_formatter =
            suite.test("planet::map::square::coordinates", [](auto check) {
                planet::map::square::coordinates const pos{7, 2};

                planet::serialise::save_buffer sb;
                planet::map::square::save(sb, pos);
                auto const bytes = sb.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "square@(7, 2)";
            });


    auto const chrono_duration_formatter = suite.test(
            "std::chrono::duration",
            [](auto check) {
                std::chrono::steady_clock::duration ns{500ns};

                planet::serialise::save_buffer ab;
                save(ab, ns);
                auto const bytes = ab.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "500ns";
            },
            [](auto check) {
                std::chrono::steady_clock::duration us{100us};

                planet::serialise::save_buffer ab;
                save(ab, us);
                auto const bytes = ab.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "100µs";
            },
            [](auto check) {
                planet::time::clock::duration ms{50ms};

                planet::serialise::save_buffer ab;
                save(ab, ms);
                auto const bytes = ab.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "50ms";
            },
            [](auto check) {
                std::chrono::system_clock::duration s{2s};

                planet::serialise::save_buffer ab;
                save(ab, s);
                auto const bytes = ab.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "2s";
            },
            [](auto check) {
                std::chrono::steady_clock::duration ns_neg{-100ns};

                planet::serialise::save_buffer ab;
                save(ab, ns_neg);
                auto const bytes = ab.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "-100ns";
            },
            [](auto check) {
                std::chrono::steady_clock::duration ms_neg{-50ms};

                planet::serialise::save_buffer ab;
                save(ab, ms_neg);
                auto const bytes = ab.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "-50ms";
            });

    /**
     * libc++ is a buggy mess with the system_clock. It gets the magnitude
     * completely wrong, so we can't do this yet.
     */
    // auto const system_clock_time_point_formatter =
    //         suite.test("std::chrono::system_clock::time_point", [](auto
    //         check) {
    //             auto const tp = std::chrono::system_clock::time_point{
    //                     std::chrono::system_clock::duration{1772888s}};
    //
    //             planet::serialise::save_buffer ab;
    //             save(ab, tp);
    //             auto const bytes = ab.complete();
    //
    //             std::ostringstream oss;
    //             planet::serialise::load_buffer lb{bytes};
    //             planet::log::pretty_print(oss, lb);
    //
    //             check(oss.str()) == "2026-03-07 12:53:20.000000";
    //         });


    auto const game_clock_formatter =
            suite.test("planet::time::clock", [](auto check) {
                planet::time::clock clock;
                clock.advance_by(50ms);

                planet::serialise::save_buffer ab;
                save(ab, clock);
                auto const bytes = ab.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "[game time +50ms]";
            });


    auto const steady_clock_formatter =
            suite.test("std::chrono::steady_clock::time_point", [](auto check) {
                /**
                 * A steady reading is resolved to its offset from the start of
                 * the run as it is saved, which is the only form of it that
                 * survives into the file.
                 */
                planet::serialise::save_buffer ab;
                save(ab,
                     planet::log::steady_clock::time_point{
                             planet::log::start_time() + 250ms});
                auto const bytes = ab.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "0.250000";
            });


    auto const optional_logging = suite.test(
            "std::optional",
            [](auto check) {
                /// A held value is logged as itself, with nothing around it
                planet::serialise::save_buffer ab;
                planet::log::detail::log(ab, std::optional<int>{42});
                auto const bytes = ab.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "42";
            },
            [](auto check) {
                /// An empty optional is the empty marker
                planet::serialise::save_buffer ab;
                planet::log::detail::log(ab, std::optional<int>{});
                auto const bytes = ab.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "empty";
            },
            [](auto check) {
                /**
                 * Because the contents go back through the log overloads, a
                 * steady reading inside an optional is resolved to its offset
                 * from the start of the run, exactly as a bare one is. It could
                 * not be saved at all.
                 */
                planet::serialise::save_buffer ab;
                planet::log::detail::log(
                        ab,
                        std::optional<std::chrono::steady_clock::time_point>{
                                planet::log::start_time() + 250ms});
                auto const bytes = ab.complete();

                std::ostringstream oss;
                planet::serialise::load_buffer lb{bytes};
                planet::log::pretty_print(oss, lb);

                check(oss.str()) == "0.250000";
            });


    auto const version_2_time_stamp =
            suite.test("file/version 2 time stamp", [](auto check) {
                /**
                 * The header of a version 2 file says when the run started on
                 * the wall clock, and the time stamps in it are counted from
                 * the start of the run, so nothing in the file has to be
                 * measured against anything else in it.
                 */
                planet::serialise::save_buffer ab;
                planet::log::write_file_headers(ab);
                save(ab,
                     planet::log::steady_clock::time_point{
                             planet::log::start_time() + 250ms});
                auto const bytes = ab.complete();

                planet::serialise::load_buffer lb{bytes};
                auto header_box = planet::serialise::expect_box(lb);
                check(header_box.version) == 2u;
                planet::log::file_header header;
                planet::log::load_fields(header_box, header);
                check(header.started.time_since_epoch().count() > 0) == true;
                check(planet::log::load_time_stamp(lb, header) == 250ms)
                        == true;
            });


    auto const version_1_time_stamp =
            suite.test("file/version 1 time stamp", [](auto check) {
                /**
                 * A version 1 file instead carries the bare steady clock
                 * readings the run took, and the one in its header is what the
                 * rest are measured against. Read that way they still come out
                 * the lengths of time they were.
                 */
                std::chrono::steady_clock::time_point const base{1000s};

                planet::serialise::save_buffer ab;
                ab.save_box_lambda(1, planet::log::file_header::box, [&]() {
                    ab.save_box(2, "_sc::time_point", base.time_since_epoch());
                    save(ab, std::string_view{"/home/someone/game"});
                });
                ab.save_box(
                        2, "_sc::time_point",
                        (base + 250ms).time_since_epoch());
                auto const bytes = ab.complete();

                planet::serialise::load_buffer lb{bytes};
                auto header_box = planet::serialise::expect_box(lb);
                planet::log::file_header header;
                planet::log::load_fields(header_box, header);
                check(header.base_time == base) == true;
                check(header.file_prefix) == "/home/someone/game";
                check(header.started.time_since_epoch().count()) == 0;
                check(planet::log::load_time_stamp(lb, header) == 250ms)
                        == true;
            });


}
