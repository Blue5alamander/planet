#include <planet/serialise.hpp>
#include <planet/telemetry/timestamps.hpp>
#include <felspar/memory/hexdump.hpp>
#include <felspar/test.hpp>


namespace {


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
