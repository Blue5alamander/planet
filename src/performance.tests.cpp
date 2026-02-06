#include <planet/telemetry/counter.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("performance/counter");


    auto const tests = suite.test("duplicate-names", [](auto check) {
        planet::telemetry::counter c1{"test-counter"};
        check([]() { planet::telemetry::counter c2{"test-counter"}; })
                .throws(std::logic_error{
                        "There is already a global performance index entry for test-counter"});
    });


}
