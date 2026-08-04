#include <planet/serialise.hpp>
#include <planet/text/configuration.hpp>

#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("text.configuration");


    auto const defaults = suite.test("defaults", [](auto check) {
        planet::text::configuration const keys;

        check(keys.commit.matches(
                {.scancode = planet::events::scancode::return_key}))
                == true;
        check(keys.cancel.matches(
                {.scancode = planet::events::scancode::escape_key}))
                == true;
        check(keys.erase_backwards.matches(
                {.scancode = planet::events::scancode::backspace_key}))
                == true;
        check(keys.erase_forwards.matches(
                {.scancode = planet::events::scancode::delete_key}))
                == true;
        check(keys.caret_left.matches(
                {.scancode = planet::events::scancode::left_key}))
                == true;
        check(keys.caret_right.matches(
                {.scancode = planet::events::scancode::right_key}))
                == true;
        check(keys.caret_to_start.matches(
                {.scancode = planet::events::scancode::home_key}))
                == true;
        check(keys.caret_to_end.matches(
                {.scancode = planet::events::scancode::end_key}))
                == true;
    });


    auto const modifiers = suite.test("default modifiers", [](auto check) {
        /**
         * Every default is the bare key, so a modifier held with it is a
         * combination the game means something else by rather than an edit
         * control.
         */
        planet::text::configuration const keys;

        check(keys.caret_to_start.matches(
                {.scancode = planet::events::scancode::home_key,
                 .modifiers = {.ctrl = true}}))
                == false;
        check(keys.commit.matches(
                {.scancode = planet::events::scancode::return_key,
                 .modifiers = {.shift = true}}))
                == false;
    });


    auto const round_trip = suite.test("serialise", [](auto check) {
        planet::text::configuration keys;
        /// A rebinding of the sort a game would save on the player's behalf
        keys.commit = {planet::events::scancode::tab_key, {}};
        keys.caret_to_end = {
                planet::events::scancode::right_key, {.ctrl = true}};

        planet::serialise::save_buffer ab;
        save(ab, keys);

        auto const loaded =
                planet::serialise::load_type<planet::text::configuration>(
                        ab.complete());

        check(loaded == keys) == true;
        check(loaded.commit.scancode == planet::events::scancode::tab_key)
                == true;
        check(loaded.caret_to_end.modifiers.ctrl) == true;
        /// The bindings that were left alone come back as they were
        check(loaded.cancel == planet::text::configuration{}.cancel) == true;
    });


    auto const defaults_round_trip =
            suite.test("serialise defaults", [](auto check) {
                planet::serialise::save_buffer ab;
                planet::text::configuration const keys;
                save(ab, keys);

                check(planet::serialise::load_type<planet::text::configuration>(
                              ab.complete())
                      == keys)
                        == true;
            });


}
