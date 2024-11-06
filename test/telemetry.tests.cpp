#include <planet/serialise.hpp>
#include <planet/telemetry/timestamps.hpp>
#include <felspar/memory/hexdump.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite =
            felspar::testsuite("timestamps", [](auto check, auto &log) {
                planet::telemetry::timestamps times{"test1"};

                times.set("first");
                check(times.is_set("first")) == true;
                check(times.is_set("second")) == false;
                times.set("second");
                check(times.is_set("second")) == true;
                times.unset("first");
                check(times.is_set("first")) == false;
                check(times.is_set("second")) == true;

                auto const first = times.times_for("first");
                auto const second = times.times_for("second");

                planet::serialise::save_buffer sb;
                save(sb, times);
                auto const bytes = sb.complete();
                log << felspar::memory::hexdump(bytes.cmemory());

                planet::serialise::load_buffer lb{bytes};
                planet::telemetry::timestamps ts{"test2"};
                load(lb, ts);
                check(ts.is_set("first")) == false;
                check(ts.is_set("second")) == true;

                check(ts.times_for("first")) == first;
                check(ts.times_for("second")) == second;
            });


}
