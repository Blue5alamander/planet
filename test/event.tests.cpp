#include <planet/serialise/events.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("events");


    auto const serialise = suite.test("serialise", []() {
        planet::serialise::save_buffer ab;

        planet::events::quit q{};
        save(ab, q);

        auto bytes{ab.complete()};
        planet::serialise::load_type<planet::events::quit>(bytes);
    });


}
