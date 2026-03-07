#include <planet/log.hpp>
#include <planet/map/hex.hpp>
#include <planet/serialise.hpp>
#include <planet/time.hpp>

#include <felspar/test.hpp>

#include <chrono>
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


}
