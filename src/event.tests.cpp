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


    auto const binding_matches =
            suite.test("key binding matches", [](auto check) {
                planet::events::key_binding const bound{
                        planet::events::scancode::letter_w,
                        {.ctrl = true}};

                check(bound.matches(
                        {.scancode = planet::events::scancode::letter_w,
                         .action = planet::events::action::down,
                         .modifiers = {.ctrl = true}}))
                        == true;
                /// The binding says nothing about the state transition
                check(bound.matches(
                        {.scancode = planet::events::scancode::letter_w,
                         .action = planet::events::action::up,
                         .modifiers = {.ctrl = true}}))
                        == true;
                check(bound.matches(
                        {.scancode = planet::events::scancode::letter_s,
                         .action = planet::events::action::down,
                         .modifiers = {.ctrl = true}}))
                        == false;
                /**
                 * A modifier the binding doesn't ask for stops the match, so
                 * ctrl-W doesn't fire when the player pressed meta-ctrl-W and
                 * meant something else by it.
                 */
                check(bound.matches(
                        {.scancode = planet::events::scancode::letter_w,
                         .action = planet::events::action::down,
                         .modifiers = {.ctrl = true, .gui = true}}))
                        == false;
                check(bound.matches(
                        {.scancode = planet::events::scancode::letter_w,
                         .action = planet::events::action::down}))
                        == false;
            });


    auto const binding_capture =
            suite.test("key binding capture", [](auto check) {
                /// What a re-binding screen does with the key it was given
                planet::events::key const pressed{
                        .scancode = planet::events::scancode::letter_q,
                        .action = planet::events::action::down,
                        .modifiers = {.shift = true}};

                auto const captured = planet::events::binding_for(pressed);

                check(captured
                      == planet::events::key_binding{
                              planet::events::scancode::letter_q,
                              {.shift = true}})
                        == true;
                check(captured.matches(pressed)) == true;
            });


    auto const binding_serialise =
            suite.test("key binding serialise", [](auto check) {
                planet::events::key_binding const bound{
                        planet::events::scancode::letter_e,
                        {.ctrl = true, .alt = true}};

                planet::serialise::save_buffer ab;
                save(ab, bound);

                check(planet::serialise::load_type<
                              planet::events::key_binding>(ab.complete())
                      == bound)
                        == true;
            });


    auto const unnamed_scancode_serialise = suite.test(
            "key binding serialise unnamed scancode", [](auto check) {
                /**
                 * The platform layer passes the hardware's scan code straight
                 * through, so a binding can hold a key the enumeration hasn't
                 * named -- F5, which is 62 in the USB HID table. Saving by
                 * number rather than by name is what keeps it.
                 */
                planet::events::key_binding const bound{
                        static_cast<planet::events::scancode>(62), {}};

                planet::serialise::save_buffer ab;
                save(ab, bound);

                check(planet::serialise::load_type<
                              planet::events::key_binding>(ab.complete())
                      == bound)
                        == true;
            });


}
