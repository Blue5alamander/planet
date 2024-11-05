#include <planet/telemetry/timestamps.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("timestamps", [](auto check) {
        planet::telemetry::timestamps times{"test1"};

        times.set("first");
        check(times.is_set("first")) == true;
        check(times.is_set("second")) == false;
        times.set("second");
        check(times.is_set("second")) == true;
        times.unset("first");
        check(times.is_set("first")) == false;
        check(times.is_set("second")) == true;
    });


}
