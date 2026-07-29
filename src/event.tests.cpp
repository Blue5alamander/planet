#include <planet/events/queue.hpp>
#include <planet/serialise/events.hpp>
#include <felspar/coro/starter.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("events");


    /**
     * Holds a consumer open on the queue for as long as it runs and records
     * everything that comes out of it, so what a forwarded queue delivers can
     * be observed.
     */
    felspar::coro::task<void>
            record(planet::queue::pmc<planet::events::text> &q,
                   std::vector<planet::events::text> &into) {
        auto values = q.values();
        while (true) { into.push_back(co_await values.next()); }
    }


    auto const text_forwarding = suite.test("text forwarding", [](auto check) {
        /**
         * What `event_loop::forward_to_baseplate` does with the text queue:
         * the whole event, timestamp included, arrives on the far side.
         */
        planet::events::queue source, sink;
        std::vector<planet::events::text> received;
        felspar::coro::starter<> tasks;
        tasks.post(record(sink.text, received));
        tasks.post(source.text.forward(sink.text));

        planet::events::text const typed{.utf8 = "ζ6"};
        source.text.push(typed);

        check(received.size()) == 1u;
        check(received.at(0).utf8) == "ζ6";
        check(received.at(0).timestamp == typed.timestamp) == true;
    });


    auto const serialise = suite.test("serialise", []() {
        planet::serialise::save_buffer ab;

        planet::events::quit q{};
        save(ab, q);

        auto bytes{ab.complete()};
        planet::serialise::load_type<planet::events::quit>(bytes);
    });


}
