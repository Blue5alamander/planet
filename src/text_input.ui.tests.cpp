#include <planet/debug/ui.hpp>
#include <planet/log.hpp>
#include <planet/ui/baseplate.hpp>
#include <planet/widget/text_input.hpp>
#include <felspar/test.hpp>


namespace {


    constexpr planet::ui::reflowable::constrained_type screen{
            {40, 0, 40}, {30, 0, 30}};


    using field_type =
            planet::widget::text_input<planet::debug::fixed_element, std::string>;


    /**
     * Bind the field to the baseplate, lay it out over a box away from the
     * origin and draw it so it joins the frame's live list. Only a drawn
     * widget can be routed an event.
     */
    void
            place(field_type &f,
                  planet::ui::baseplate &bp,
                  planet::ui::panel &panel) {
        f.add_to(bp, panel);
        f.reflow({.screen = screen}, screen);
        f.move_to(
                {.screen = screen},
                {{15, 20}, planet::affine::extents2d{4, 3}});
        f.draw();
    }


    /// Put the pointer over the field without pressing anything.
    void hover(planet::ui::baseplate &bp) {
        bp.events.mouse.push({.location = {16, 21}});
    }
    /**
     * Move the pointer off the field. Nothing is under it there, so the soft
     * focus is dropped and whatever `has_focus` still reports can only be the
     * hard focus.
     */
    void pointer_away(planet::ui::baseplate &bp) {
        bp.events.mouse.push({.location = {1, 1}});
    }
    /// Left click over the field.
    void click(planet::ui::baseplate &bp) {
        bp.events.mouse.push(
                {.button = planet::events::button::left,
                 .action = planet::events::action::down,
                 .location = {16, 21}});
        bp.events.mouse.push(
                {.button = planet::events::button::left,
                 .action = planet::events::action::up,
                 .location = {16, 21},
                 .clicks = 1});
    }
    void type(planet::ui::baseplate &bp, std::string s) {
        bp.events.text.push({.utf8 = std::move(s)});
    }
    void press(planet::ui::baseplate &bp, planet::events::scancode const sc) {
        bp.events.key.push(
                {.scancode = sc, .action = planet::events::action::down});
    }


    auto const suite = felspar::testsuite("text_input", []() {
        planet::log::active = planet::log::level::error;
    });


    auto const subscriptions =
            suite.test("subscriptions", [](auto check, auto &log) {
                /**
                 * Delivery tests `consumer_count()` on the queue for the kind,
                 * so the field has to hold all three open at rest or the kinds
                 * it is not currently blocked on fall through to whatever it
                 * covers.
                 */
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                std::string output{"Nomad"};
                field_type field{
                        "field", planet::debug::fixed_element{log, {4, 3}},
                        output, "Nomad"};
                field.add_to(bp, panel);

                check(field.events.mouse.consumer_count()) == 1u;
                check(field.events.key.consumer_count()) == 1u;
                check(field.events.text.consumer_count()) == 1u;
            });


    auto const activation = suite.test("activation", [](auto check, auto &log) {
        planet::ui::baseplate bp;
        planet::ui::panel panel;
        std::string output{"Nomad"};
        field_type field{
                "field", planet::debug::fixed_element{log, {4, 3}}, output,
                "Nomad"};
        place(field, bp, panel);

        check(field.is_editing()) == false;

        click(bp);

        check(field.is_editing()) == true;
        /// Phase 1 retypes rather than corrects, so the buffer is empty
        check(field.value()) == "";

        pointer_away(bp);
        check(bp.has_focus(field)) == true;
    });


    auto const typing = suite.test("typing", [](auto check, auto &log) {
        planet::ui::baseplate bp;
        planet::ui::panel panel;
        std::string output{"Nomad"};
        field_type field{
                "field", planet::debug::fixed_element{log, {4, 3}}, output,
                "Nomad"};
        place(field, bp, panel);

        click(bp);
        type(bp, "Ze");
        type(bp, "ta");

        check(field.value()) == "Zeta";
        /// Nothing reaches the output until the edit is committed
        check(output) == "Nomad";
    });


    auto const committing =
            suite.test("return commits", [](auto check, auto &log) {
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                std::string output{"Nomad"};
                field_type field{
                        "field", planet::debug::fixed_element{log, {4, 3}},
                        output, "Nomad"};
                place(field, bp, panel);

                click(bp);
                type(bp, "Vagrant");
                press(bp, planet::events::scancode::return_key);

                check(output) == "Vagrant";
                check(field.value()) == "Vagrant";
                check(field.is_editing()) == false;

                /// The hard focus was given up, so normal routing is restored
                pointer_away(bp);
                check(bp.has_focus(field)) == false;
            });


    auto const cancelling =
            suite.test("escape cancels", [](auto check, auto &log) {
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                std::string output{"Nomad"};
                field_type field{
                        "field", planet::debug::fixed_element{log, {4, 3}},
                        output, "Nomad"};
                place(field, bp, panel);

                click(bp);
                type(bp, "Vagrant");
                press(bp, planet::events::scancode::escape_key);

                check(output) == "Nomad";
                check(field.value()) == "Nomad";
                check(field.is_editing()) == false;

                pointer_away(bp);
                check(bp.has_focus(field)) == false;
            });


    auto const ignored_at_rest =
            suite.test("text ignored at rest", [](auto check, auto &log) {
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                std::string output{"Nomad"};
                field_type field{
                        "field", planet::debug::fixed_element{log, {4, 3}},
                        output, "Nomad"};
                place(field, bp, panel);

                hover(bp);
                type(bp, "x");

                /**
                 * The field is subscribed, so the text really was delivered to
                 * it -- it is the field that discards it rather than the
                 * routing never reaching it.
                 */
                check(field.events.text.values_pushed()) == 1u;
                check(field.value()) == "Nomad";
                check(output) == "Nomad";
            });


    auto const edit_state = suite.test(
            "edit state callback",
            [](auto check, auto &log) {
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                std::string output{"Nomad"};
                field_type field{
                        "field", planet::debug::fixed_element{log, {4, 3}},
                        output, "Nomad"};
                place(field, bp, panel);
                /// `+` for a begin-edit, `-` for an end-edit
                std::string transitions;
                field.editing_changed = [&](bool const e) {
                    transitions += (e ? '+' : '-');
                };

                click(bp);
                check(transitions) == "+";

                press(bp, planet::events::scancode::return_key);
                check(transitions) == "+-";
            },
            [](auto check, auto &log) {
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                std::string output{"Nomad"};
                field_type field{
                        "field", planet::debug::fixed_element{log, {4, 3}},
                        output, "Nomad"};
                place(field, bp, panel);
                std::string transitions;
                field.editing_changed = [&](bool const e) {
                    transitions += (e ? '+' : '-');
                };

                click(bp);
                press(bp, planet::events::scancode::escape_key);

                /// Cancelling ends the edit just as committing does
                check(transitions) == "+-";
            });


}
