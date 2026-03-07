#include <planet/log.hpp>
#include <planet/map/hex.hpp>
#include <planet/serialise.hpp>

#include <felspar/test.hpp>

#include <sstream>


namespace {


    auto const suite = felspar::testsuite("log");


    /// ## Test hex coordinates log formatter
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


    /// ## Test square coordinates log formatter
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


}
