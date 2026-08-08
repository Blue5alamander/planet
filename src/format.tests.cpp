#include <planet/time/format.hpp>
#include <felspar/test.hpp>


using namespace std::literals;


namespace {


    auto const suite = felspar::testsuite("time.format");


    auto const display = suite.test(
            "display_string",
            [](auto check) {
                // Below a microsecond the nanosecond count is verbatim
                check(planet::time::display_string(0ns)) == "0ns";
                check(planet::time::display_string(1ns)) == "1ns";
                check(planet::time::display_string(999ns)) == "999ns";
            },
            [](auto check) {
                // Three significant figures across the µs decade
                check(planet::time::display_string(1'000ns)) == "1.00µs";
                check(planet::time::display_string(1'450ns)) == "1.45µs";
                check(planet::time::display_string(14'500ns)) == "14.5µs";
                check(planet::time::display_string(145'000ns)) == "145µs";
                check(planet::time::display_string(999'000ns)) == "999µs";
            },
            [](auto check) {
                // ...and the ms and s decades read the same way
                check(planet::time::display_string(1'450'000ns)) == "1.45ms";
                check(planet::time::display_string(14'500us)) == "14.5ms";
                check(planet::time::display_string(145ms)) == "145ms";
                check(planet::time::display_string(1'450ms)) == "1.45s";
                check(planet::time::display_string(14'500ms)) == "14.5s";
                check(planet::time::display_string(99'900ms)) == "99.9s";
            },
            [](auto check) {
                // Rounding carries up and across unit boundaries
                check(planet::time::display_string(1'455ns)) == "1.46µs";
                check(planet::time::display_string(999'600ns)) == "1.00ms";
                check(planet::time::display_string(999'600us)) == "1.00s";
            },
            [](auto check) {
                // From 100s the day/hour/minute/second breakdown kicks in
                check(planet::time::display_string(100s)) == "1m40s";
                check(planet::time::display_string(3'661s)) == "1h1m1s";
                check(planet::time::display_string(93'825s)) == "1d2h3m45s";
                check(planet::time::display_string(86'405s)) == "1d0h0m5s";
                check(planet::time::display_string(200'000h)) == "8333d8h0m0s";
            },
            [](auto check) {
                // The sign rides along in every band
                check(planet::time::display_string(-999ns)) == "-999ns";
                check(planet::time::display_string(-1'450ns)) == "-1.45µs";
                check(planet::time::display_string(-1'450ms)) == "-1.45s";
                check(planet::time::display_string(-3'661s)) == "-1h1m1s";
            });


}
