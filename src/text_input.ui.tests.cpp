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
     * Lay the field out over a box away from the origin and draw it so it
     * joins the frame's live list. Only a drawn widget can be routed an event.
     */
    void lay_out(field_type &f) {
        f.reflow({.screen = screen}, screen);
        f.move_to(
                {.screen = screen},
                {{15, 20}, planet::affine::extents2d{4, 3}});
        f.draw();
    }
    /// Bind the field straight to the baseplate and lay it out.
    void
            place(field_type &f,
                  planet::ui::baseplate &bp,
                  planet::ui::panel &panel) {
        f.add_to(bp, panel);
        lay_out(f);
    }
    /// Bind the field above another widget -- a modal's screen -- and lay it out.
    void place_in(field_type &f, planet::ui::widget &parent) {
        f.add_to(parent);
        lay_out(f);
    }


    /**
     * Put a button on the display well away from the field, so that a click
     * over it is unambiguously a click aimed at something other than the field
     * being edited.
     */
    void place_away(
            planet::debug::button<> &b,
            planet::ui::baseplate &bp,
            planet::ui::panel &panel) {
        b.add_to(bp, panel);
        b.reflow({.screen = screen}, screen);
        b.move_to(
                {.screen = screen}, {{1, 1}, planet::affine::extents2d{4, 3}});
        b.draw();
    }


    /**
     * Draw a frame. A widget only joins the routing by drawing itself, so the
     * field's dismissal screen is not there to take a click until a frame has
     * been drawn with the edit already running.
     */
    void
            frame(planet::ui::baseplate &bp,
                  std::initializer_list<planet::ui::widget *> const ws) {
        bp.start_frame_reset();
        for (auto *const w : ws) { w->draw(); }
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
    /// Left click at a location.
    void click_at(planet::ui::baseplate &bp, planet::affine::point2d const l) {
        bp.events.mouse.push(
                {.button = planet::events::button::left,
                 .action = planet::events::action::down,
                 .location = l});
        bp.events.mouse.push(
                {.button = planet::events::button::left,
                 .action = planet::events::action::up,
                 .location = l,
                 .clicks = 1});
    }
    /// Left click over the field.
    void click(planet::ui::baseplate &bp) { click_at(bp, {16, 21}); }
    /// Left click over the button placed away from the field.
    void click_elsewhere(planet::ui::baseplate &bp) { click_at(bp, {2, 2}); }
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


    auto const click_away = suite.test(
            "click away",
            [](auto check, auto &log) {
                /**
                 * The field holds the hard focus while it is editing, so it is
                 * handed every click in the window whatever it was aimed at,
                 * and passes each one down to its dismissal screen. The screen
                 * ends the edit, keeping what was typed.
                 */
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                std::string output{"Nomad"};
                field_type field{
                        "field", planet::debug::fixed_element{log, {4, 3}},
                        output, "Nomad"};
                place(field, bp, panel);
                planet::debug::button<> elsewhere{log};
                place_away(elsewhere, bp, panel);

                click(bp);
                type(bp, "Vagrant");
                frame(bp, {&field, &elsewhere});
                click_elsewhere(bp);

                check(field.is_editing()) == false;
                check(output) == "Vagrant";
                check(field.value()) == "Vagrant";
                check(bp.has_focus(field)) == false;
            },
            [](auto check, auto &log) {
                /**
                 * The screen is a hover boundary, so the click that ends the
                 * edit stops there: the button under the pointer is dismissed
                 * past, not pressed. Pressing it takes a second click, once
                 * the edit is over.
                 */
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                std::string output{"Nomad"};
                field_type field{
                        "field", planet::debug::fixed_element{log, {4, 3}},
                        output, "Nomad"};
                place(field, bp, panel);
                planet::debug::button<> elsewhere{log};
                place_away(elsewhere, bp, panel);

                click(bp);
                frame(bp, {&field, &elsewhere});
                click_elsewhere(bp);

                check(elsewhere.clicks) == 0u;

                frame(bp, {&field, &elsewhere});
                click_elsewhere(bp);

                check(elsewhere.clicks) == 1u;
            },
            [](auto check, auto &log) {
                /**
                 * Ending the edit by clicking away is an end-edit like any
                 * other, so the platform's text input -- and with it a mobile
                 * on-screen keyboard -- is switched off.
                 */
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                std::string output{"Nomad"};
                field_type field{
                        "field", planet::debug::fixed_element{log, {4, 3}},
                        output, "Nomad"};
                place(field, bp, panel);
                planet::debug::button<> elsewhere{log};
                place_away(elsewhere, bp, panel);
                std::string transitions;
                field.editing_changed = [&](bool const e) {
                    transitions += (e ? '+' : '-');
                };

                click(bp);
                frame(bp, {&field, &elsewhere});
                click_elsewhere(bp);

                check(transitions) == "+-";
            },
            [](auto check, auto &log) {
                /**
                 * At rest the screen is not drawn, so it is not in the routing
                 * at all and a click on another widget reaches it as usual.
                 */
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                std::string output{"Nomad"};
                field_type field{
                        "field", planet::debug::fixed_element{log, {4, 3}},
                        output, "Nomad"};
                place(field, bp, panel);
                planet::debug::button<> elsewhere{log};
                place_away(elsewhere, bp, panel);

                click_elsewhere(bp);

                check(field.is_editing()) == false;
                check(elsewhere.clicks) == 1u;
            },
            [](auto check, auto &log) {
                /**
                 * A field inside a modal is attached above the modal's own
                 * screen, so its dismissal screen has to be placed above that
                 * one too. Were it at a fixed layer instead the click that
                 * ends an edit would walk past it to the modal's screen, which
                 * would close the whole modal and leave the edit running.
                 */
                planet::ui::baseplate bp;
                planet::ui::screen modal{"modal", 100.0f};
                modal.add_to(bp);
                modal.draw();

                std::string output{"Nomad"};
                field_type field{
                        "field", planet::debug::fixed_element{log, {4, 3}},
                        output, "Nomad"};
                place_in(field, modal);

                click(bp);
                check(field.is_editing()) == true;

                type(bp, "Vagrant");
                frame(bp, {&modal, &field});
                click_elsewhere(bp);

                check(field.is_editing()) == false;
                check(output) == "Vagrant";
                /**
                 * The modal screen is a hover boundary, so anything that got
                 * as far as it would have been pushed to its queue whether it
                 * subscribes or not. Nothing did.
                 */
                check(modal.events.mouse.values_pushed()) == 0u;
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
