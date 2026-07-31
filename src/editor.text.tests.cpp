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
        /**
         * An edit corrects rather than retypes: what is there is what is being
         * edited, with the caret at the end of it ready for more.
         */
        check(ed.value()) == "Nomad";
        check(ed.cursor()) == 5u;
    });


    auto const typing = suite.test(
            "typing",
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();

                check(ed.handle(typed(" Ze")))
                        == planet::text::outcome::changed;
                check(ed.handle(typed("ta"))) == planet::text::outcome::changed;

                check(ed.value()) == "Nomad Zeta";
                check(ed.cursor()) == 10u;
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
                /// A single line has nowhere for up and down to go either
                check(ed.handle(down(planet::events::scancode::up_key)))
                        == planet::text::outcome::ignored;
                check(ed.handle(down(planet::events::scancode::down_key)))
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
        check(ed.handle(down(planet::events::scancode::delete_key)))
                == planet::text::outcome::ignored;
        /**
         * Navigation is ignored at rest as well, caret and all: a field is
         * subscribed to keys whether or not it is being edited, and the arrow
         * keys belong to whatever else is listening for them until an edit
         * takes them.
         */
        check(ed.handle(down(planet::events::scancode::left_key)))
                == planet::text::outcome::ignored;
        check(ed.handle(down(planet::events::scancode::right_key)))
                == planet::text::outcome::ignored;
        check(ed.handle(down(planet::events::scancode::home_key)))
                == planet::text::outcome::ignored;
        check(ed.handle(down(planet::events::scancode::end_key)))
                == planet::text::outcome::ignored;

        check(ed.is_editing()) == false;
        check(ed.value()) == "Nomad";
        check(ed.cursor()) == 0u;
    });


    auto const backspace = suite.test(
            "backspace",
            [](auto check) {
                planet::text::editor ed{"Zeta"};
                ed.begin();

                check(ed.handle(down(planet::events::scancode::backspace_key)))
                        == planet::text::outcome::changed;

                check(ed.value()) == "Zet";
                /// The caret follows the text it took out
                check(ed.cursor()) == 3u;
            },
            [](auto check) {
                /// Nothing to remove is nothing changed
                planet::text::editor ed{};
                ed.begin();

                check(ed.handle(down(planet::events::scancode::backspace_key)))
                        == planet::text::outcome::ignored;

                check(ed.value()) == "";
            },
            [](auto check) {
                /// Nor is there anything before a caret at the start of the text
                planet::text::editor ed{"Zeta"};
                ed.begin();
                ed.handle(down(planet::events::scancode::home_key));

                check(ed.handle(down(planet::events::scancode::backspace_key)))
                        == planet::text::outcome::ignored;

                check(ed.value()) == "Zeta";
                check(ed.cursor()) == 0u;
            },
            [](auto check) {
                /// Correcting a typo and carrying on
                planet::text::editor ed{"Zetz"};
                ed.begin();
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
                planet::text::editor ed{"Zeté"};
                ed.begin();

                check(ed.handle(down(planet::events::scancode::backspace_key)))
                        == planet::text::outcome::changed;

                check(ed.value()) == "Zet";
                check(ed.cursor()) == 3u;
            },
            [](auto check) {
                planet::text::editor ed{"Ze🚀"};
                ed.begin();

                ed.handle(down(planet::events::scancode::backspace_key));

                check(ed.value()) == "Ze";
                check(ed.cursor()) == 2u;
            },
            [](auto check) {
                /// Backspace acts at the caret, not at the end of the text
                planet::text::editor ed{"Zeta"};
                ed.begin();
                ed.handle(down(planet::events::scancode::left_key));

                ed.handle(down(planet::events::scancode::backspace_key));

                check(ed.value()) == "Zea";
                check(ed.cursor()) == 2u;
            },
            [](auto check) {
                /// A key coming back up is not a key press, backspace included
                planet::text::editor ed{"Zeta"};
                ed.begin();

                check(ed.handle(up(planet::events::scancode::backspace_key)))
                        == planet::text::outcome::ignored;

                check(ed.value()) == "Zeta";
            });


    auto const navigation = suite.test(
            "navigation",
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();

                check(ed.handle(down(planet::events::scancode::left_key)))
                        == planet::text::outcome::changed;
                check(ed.cursor()) == 4u;
                check(ed.handle(down(planet::events::scancode::right_key)))
                        == planet::text::outcome::changed;
                check(ed.cursor()) == 5u;

                /// Neither arrow runs off the end of the buffer
                check(ed.handle(down(planet::events::scancode::right_key)))
                        == planet::text::outcome::ignored;
                check(ed.cursor()) == 5u;

                check(ed.value()) == "Nomad";
            },
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();

                check(ed.handle(down(planet::events::scancode::home_key)))
                        == planet::text::outcome::changed;
                check(ed.cursor()) == 0u;
                check(ed.handle(down(planet::events::scancode::left_key)))
                        == planet::text::outcome::ignored;
                check(ed.cursor()) == 0u;

                check(ed.handle(down(planet::events::scancode::end_key)))
                        == planet::text::outcome::changed;
                check(ed.cursor()) == 5u;
                /// Already there is nothing changed
                check(ed.handle(down(planet::events::scancode::end_key)))
                        == planet::text::outcome::ignored;
            },
            [](auto check) {
                /**
                 * The caret steps over whole characters, so it can never land
                 * inside one: `→` is three bytes, between two of one.
                 */
                planet::text::editor ed{"a→b"};
                ed.begin();

                check(ed.cursor()) == 5u;
                ed.handle(down(planet::events::scancode::left_key));
                check(ed.cursor()) == 4u;
                ed.handle(down(planet::events::scancode::left_key));
                check(ed.cursor()) == 1u;
                ed.handle(down(planet::events::scancode::left_key));
                check(ed.cursor()) == 0u;

                ed.handle(down(planet::events::scancode::right_key));
                check(ed.cursor()) == 1u;
                ed.handle(down(planet::events::scancode::right_key));
                check(ed.cursor()) == 4u;
            });


    auto const insertion = suite.test(
            "insertion",
            [](auto check) {
                /// Typing goes in at the caret and the caret follows it along
                planet::text::editor ed{"Nomad"};
                ed.begin();
                ed.handle(down(planet::events::scancode::home_key));

                check(ed.handle(typed("The ")))
                        == planet::text::outcome::changed;

                check(ed.value()) == "The Nomad";
                check(ed.cursor()) == 4u;

                ed.handle(typed("Old "));

                check(ed.value()) == "The Old Nomad";
                check(ed.cursor()) == 8u;
            },
            [](auto check) {
                /// The caret advances by the bytes inserted, not by characters
                planet::text::editor ed{"Zet"};
                ed.begin();

                ed.handle(typed("é"));

                check(ed.value()) == "Zeté";
                check(ed.cursor()) == 5u;
            });


    auto const forward_delete = suite.test(
            "delete",
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();
                ed.handle(down(planet::events::scancode::home_key));

                check(ed.handle(down(planet::events::scancode::delete_key)))
                        == planet::text::outcome::changed;

                check(ed.value()) == "omad";
                /// The text moves back to the caret, so the caret stays put
                check(ed.cursor()) == 0u;
            },
            [](auto check) {
                /// Nothing follows a caret at the end of the text
                planet::text::editor ed{"Nomad"};
                ed.begin();

                check(ed.handle(down(planet::events::scancode::delete_key)))
                        == planet::text::outcome::ignored;

                check(ed.value()) == "Nomad";
            },
            [](auto check) {
                /// A whole character goes here too
                planet::text::editor ed{"→b"};
                ed.begin();
                ed.handle(down(planet::events::scancode::home_key));

                ed.handle(down(planet::events::scancode::delete_key));

                check(ed.value()) == "b";
                check(ed.cursor()) == 0u;
            });


    auto const cursor_from_outside = suite.test(
            "set_cursor",
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();

                ed.set_cursor(2);
                check(ed.cursor()) == 2u;
                ed.set_cursor(0);
                check(ed.cursor()) == 0u;
                /**
                 * Past the end is the end, not the boundary before it -- a
                 * click past the last character puts the caret after it.
                 */
                ed.set_cursor(99);
                check(ed.cursor()) == 5u;
            },
            [](auto check) {
                /**
                 * A presentation measures its way to an offset and can land
                 * inside a character. What it gets is the boundary at or before
                 * where it asked, never somewhere illegal.
                 */
                planet::text::editor ed{"a→b"};
                ed.begin();

                ed.set_cursor(2);
                check(ed.cursor()) == 1u;
                ed.set_cursor(3);
                check(ed.cursor()) == 1u;
                ed.set_cursor(4);
                check(ed.cursor()) == 4u;
            },
            [](auto check) {
                /// So what is typed after one is still valid text
                planet::text::editor ed{"a→b"};
                ed.begin();
                ed.set_cursor(3);

                ed.handle(typed("x"));

                check(ed.value()) == "ax→b";
            });


    auto const ending = suite.test(
            "ending an edit",
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();
                ed.handle(typed(" II"));

                ed.commit();

                check(ed.value()) == "Nomad II";
                check(ed.is_editing()) == false;
            },
            [](auto check) {
                planet::text::editor ed{"Nomad"};
                ed.begin();
                ed.handle(typed(" II"));

                ed.cancel();

                check(ed.value()) == "Nomad";
                check(ed.is_editing()) == false;
                /**
                 * The caret comes back inside the value that was put back: it
                 * was past the end of it a moment ago, and every offset the
                 * editor holds has to be one into the text it holds now.
                 */
                check(ed.cursor()) == 5u;
            });


    auto const filtering = suite.test(
            "what the value is allowed to be",
            [](auto check) {
                /**
                 * A cap on how long the value may grow. The filter is handed
                 * the value as it *would* be, so a rule about the whole of it
                 * is written as a rule about the whole of it.
                 */
                planet::text::editor ed{"Nomad"};
                ed.acceptable = [](std::string_view const v) {
                    return v.size() <= 7;
                };
                ed.begin();

                check(ed.handle(typed(" I"))) == planet::text::outcome::changed;
                check(ed.value()) == "Nomad I";
                /**
                 * The insertion that would take it past the cap does not happen
                 * at all: the buffer and the caret are exactly as they were,
                 * and the editor reports that nothing changed so no redraw is
                 * charged for a keystroke that did nothing.
                 */
                check(ed.handle(typed("I"))) == planet::text::outcome::ignored;
                check(ed.value()) == "Nomad I";
                check(ed.cursor()) == 7u;
            },
            [](auto check) {
                /**
                 * A refused insertion at the caret leaves what is on either
                 * side of it alone, not just what is on the end.
                 */
                planet::text::editor ed{"Nomad"};
                ed.acceptable = [](std::string_view const v) {
                    return v.find('x') == std::string_view::npos;
                };
                ed.begin();
                ed.handle(down(planet::events::scancode::home_key));

                check(ed.handle(typed("x"))) == planet::text::outcome::ignored;

                check(ed.value()) == "Nomad";
                check(ed.cursor()) == 0u;
            },
            [](auto check) {
                /**
                 * The whole of what was typed goes back, however many bytes it
                 * took: what a refused insertion leaves behind has to still be
                 * the text that was there before it.
                 */
                planet::text::editor ed{"a"};
                ed.acceptable = [](std::string_view const v) {
                    return v.size() < 4;
                };
                ed.begin();

                check(ed.handle(typed("→"))) == planet::text::outcome::ignored;

                check(ed.value()) == "a";
                check(ed.cursor()) == 1u;
            },
            [](auto check) {
                /**
                 * Deletion is not filtered. A value can always be cut down to
                 * nothing -- a cap that stopped it being cut back would be no
                 * use, and a rule that it may not be blank has to leave a way
                 * to clear it and start again.
                 */
                planet::text::editor ed{"Nomad"};
                ed.acceptable = [](std::string_view const v) {
                    return not v.empty();
                };
                ed.begin();
                while (not ed.value().empty()) {
                    ed.handle(down(planet::events::scancode::backspace_key));
                }
                check(ed.value()) == "";

                /// What it will not do is let the edit *end* on one
                check(ed.commit()) == false;

                check(ed.value()) == "Nomad";
                check(ed.is_editing()) == false;
                check(ed.cursor()) == 5u;
            },
            [](auto check) {
                /// A value the filter is happy with commits as it always did
                planet::text::editor ed{"Nomad"};
                ed.acceptable = [](std::string_view const v) {
                    return not v.empty();
                };
                ed.begin();
                ed.handle(typed(" II"));

                check(ed.commit()) == true;

                check(ed.value()) == "Nomad II";
                check(ed.is_editing()) == false;
            });


}
