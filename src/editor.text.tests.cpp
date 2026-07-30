#include <planet/text/editor.hpp>

#include <felspar/test.hpp>


namespace {


    planet::events::text typed(std::string s) { return {.utf8 = std::move(s)}; }
    planet::events::key down(planet::events::scancode const sc) {
        return {.scancode = sc, .action = planet::events::action::down};
    }
    planet::events::key up(planet::events::scancode const sc) {
        return {.scancode = sc, .action = planet::events::action::up};
    }


    auto const suite = felspar::testsuite("text.editor");


    auto const at_rest = suite.test("at rest", [](auto check) {
        planet::text::editor ed{"Nomad"};

        check(ed.is_editing()) == false;
        check(ed.value()) == "Nomad";
    });


    auto const beginning = suite.test("begin", [](auto check) {
        planet::text::editor ed{"Nomad"};

        ed.begin();

        check(ed.is_editing()) == true;
        /// An edit retypes rather than corrects, so the buffer starts empty
        check(ed.value()) == "";
    });


    auto const typing = suite.test(
            "typing",
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();

                check(ed.handle(typed("Ze"))) == planet::text::outcome::changed;
                check(ed.handle(typed("ta"))) == planet::text::outcome::changed;

                check(ed.value()) == "Zeta";
            },
            [](auto check) {
                /**
                 * Text arrives at a field whenever it is subscribed, which is
                 * all of the time, so an editor at rest has to say that it
                 * wanted none of it.
                 */
                planet::text::editor ed{"Nomad"};

                check(ed.handle(typed("x"))) == planet::text::outcome::ignored;

                check(ed.value()) == "Nomad";
            });


    auto const control_keys = suite.test(
            "control keys",
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();

                check(ed.handle(down(planet::events::scancode::return_key)))
                        == planet::text::outcome::commit;
                check(ed.handle(down(planet::events::scancode::escape_key)))
                        == planet::text::outcome::cancel;
                check(ed.handle(down(planet::events::scancode::letter_a)))
                        == planet::text::outcome::ignored;
                /**
                 * Reporting an outcome is not acting on it: the side effects of
                 * ending an edit need a baseplate, so they belong to the shell,
                 * and the editor is still editing until it is told otherwise.
                 */
                check(ed.is_editing()) == true;
            },
            [](auto check) {
                /**
                 * Only the key going down ends an edit. A key still held from
                 * before the edit began releases into it, and that release must
                 * not be what finishes it.
                 */
                planet::text::editor ed{"Nomad"};
                ed.begin();

                check(ed.handle(up(planet::events::scancode::return_key)))
                        == planet::text::outcome::ignored;
                check(ed.handle(up(planet::events::scancode::escape_key)))
                        == planet::text::outcome::ignored;
            });


    auto const keys_at_rest = suite.test("keys at rest", [](auto check) {
        planet::text::editor ed{"Nomad"};

        check(ed.handle(down(planet::events::scancode::return_key)))
                == planet::text::outcome::ignored;
        check(ed.handle(down(planet::events::scancode::escape_key)))
                == planet::text::outcome::ignored;
        check(ed.handle(down(planet::events::scancode::letter_a)))
                == planet::text::outcome::ignored;
        check(ed.handle(down(planet::events::scancode::backspace_key)))
                == planet::text::outcome::ignored;

        check(ed.is_editing()) == false;
        check(ed.value()) == "Nomad";
    });


    auto const backspace = suite.test(
            "backspace",
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();
                ed.handle(typed("Zeta"));

                check(ed.handle(down(planet::events::scancode::backspace_key)))
                        == planet::text::outcome::changed;

                check(ed.value()) == "Zet";
            },
            [](auto check) {
                /// Nothing to remove is nothing changed
                planet::text::editor ed{"Nomad"};
                ed.begin();

                check(ed.handle(down(planet::events::scancode::backspace_key)))
                        == planet::text::outcome::ignored;

                check(ed.value()) == "";
            },
            [](auto check) {
                /// Correcting a typo and carrying on
                planet::text::editor ed{"Nomad"};
                ed.begin();
                ed.handle(typed("Zetz"));
                ed.handle(down(planet::events::scancode::backspace_key));
                ed.handle(typed("a"));

                ed.commit();

                check(ed.value()) == "Zeta";
            },
            [](auto check) {
                /**
                 * A whole character goes, never the tail byte of one: what is
                 * left has to still be text.
                 */
                planet::text::editor ed{"Nomad"};
                ed.begin();
                ed.handle(typed("Zeté"));

                check(ed.handle(down(planet::events::scancode::backspace_key)))
                        == planet::text::outcome::changed;

                check(ed.value()) == "Zet";
            },
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();
                ed.handle(typed("Ze🚀"));

                ed.handle(down(planet::events::scancode::backspace_key));

                check(ed.value()) == "Ze";
            },
            [](auto check) {
                /// A key coming back up is not a key press, backspace included
                planet::text::editor ed{"Nomad"};
                ed.begin();
                ed.handle(typed("Zeta"));

                check(ed.handle(up(planet::events::scancode::backspace_key)))
                        == planet::text::outcome::ignored;

                check(ed.value()) == "Zeta";
            });


    auto const ending = suite.test(
            "ending an edit",
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();
                ed.handle(typed("Vagrant"));

                ed.commit();

                check(ed.value()) == "Vagrant";
                check(ed.is_editing()) == false;
            },
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();
                ed.handle(typed("Vagrant"));

                ed.cancel();

                check(ed.value()) == "Nomad";
                check(ed.is_editing()) == false;
            });


}
