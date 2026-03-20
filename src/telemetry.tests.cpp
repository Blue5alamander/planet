#include <planet/serialise.hpp>
#include <planet/telemetry/duration.hpp>
#include <planet/telemetry/timestamps.hpp>
#include <felspar/memory/hexdump.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite_steady_duration = felspar::testsuite(
            "steady_duration",
            [](auto check) {
                using namespace std::chrono_literals;
                planet::telemetry::steady_duration d{"test_sd_single", 16};
                d.add_measurement(1ms);
                check(d.value().count()) > 0;
            },
            [](auto check) {
                using namespace std::chrono_literals;
                planet::telemetry::steady_duration d{"test_sd_converge", 4};
                for (int i = 0; i < 100; ++i) { d.add_measurement(1ms); }
                // After many readings with half-life=4, value should be close
                // to 1ms (within 10%)
                auto const v = d.value();
                check(v.count()) >= 900'000;
                check(v.count()) <= 1'100'000;
            },
            [](auto check) {
                using namespace std::chrono_literals;
                auto bytes = []() {
                    planet::telemetry::steady_duration d{"test_sd_save", 8};
                    d.add_measurement(2ms);
                    planet::serialise::save_buffer sb;
                    planet::telemetry::save_performance(sb, d);
                    return sb.complete();
                }();
                planet::serialise::load_buffer lb{bytes};
                planet::telemetry::steady_duration d2{"test_sd_save", 8};
                planet::telemetry::load_performance(lb, d2);
                check(d2.value().count()) > 0;
            });


    auto const suite =
            felspar::testsuite("timestamps", [](auto check, auto &log) {
                auto [bytes, first, second] = [&]() {
                    planet::telemetry::timestamps times{"test1"};

                    times.set("first");
                    check(times.is_set("first")) == true;
                    check(times.is_set("second")) == false;
                    times.set("second");
                    check(times.is_set("second")) == true;
                    times.unset("first");
                    check(times.is_set("first")) == false;
                    check(times.is_set("second")) == true;

                    planet::serialise::save_buffer sb;
                    planet::telemetry::save_performance(sb, times);
                    return std::tuple{
                            sb.complete(), times.times_for("first"),
                            times.times_for("second")};
                }();
                log << felspar::memory::hexdump(bytes.cmemory());

                planet::serialise::load_buffer lb{bytes};
                planet::telemetry::timestamps ts{"test1"};
                planet::telemetry::load_performance(lb, ts);
                check(ts.is_set("first")) == false;
                check(ts.is_set("second")) == true;

                check(ts.times_for("first")) == first;
                check(ts.times_for("second")) == second;
            });


}
