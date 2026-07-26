#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/range.hpp>
#include <planet/ui/screen.hpp>
#include <felspar/test.hpp>


namespace {


    constexpr planet::ui::reflowable::constrained_type screen_constraints = {
            {50, 0, 100}, {50, 0, 100}};


    auto const suite = felspar::testsuite("range.ui");


    auto const reflow = suite.test("reflow", [](auto check, auto &log) {
        planet::ui::baseplate bp;
        auto r = planet::ui::range{
                planet::debug::fixed_element{log, {50, 10}},
                planet::ui::draggable<planet::debug::fixed_element>{
                        "hs", planet::debug::fixed_element{log, {10, 10}}},
                {20, 0, 100}};
        r.add_to(bp);
        r.reflow({.screen = screen_constraints}, screen_constraints);
        r.move_to(
                {.screen = screen_constraints},
                {{15, 30}, planet::affine::extents2d{50, 50}});
        r.draw();

        check(r.wants_focus()) == true;
        check(r.slider.wants_focus()) == true;

        check(r.position())
                == planet::affine::rectangle2d{
                        {15, 30}, planet::affine::extents2d{50, 10}};

        check(r.contains_global_coordinate(planet::affine::point2d{16, 21}))
                == false;
        check(r.contains_global_coordinate(planet::affine::point2d{16, 31}))
                == true;
        check(r.contains_global_coordinate(planet::affine::point2d{16, 41}))
                == false;
        /// With no pointer at all an ordinary widget contains nothing
        check(r.contains_global_coordinate({})) == false;

        check(r.slider.offset.min_extents()) == planet::affine::extents2d{0, 0};
        check(r.slider.offset.extents()) == planet::affine::extents2d{8, 0};
        check(r.slider.offset.max_extents())
                == planet::affine::extents2d{40, 0};

        check(r.slider.position())
                == planet::affine::rectangle2d{
                        {23, 30}, planet::affine::extents2d{10, 10}};

        check(r.slider.contains_global_coordinate(
                planet::affine::point2d{24, 21}))
                == false;
        check(r.slider.contains_global_coordinate(
                planet::affine::point2d{24, 31}))
                == true;
        check(r.slider.contains_global_coordinate(
                planet::affine::point2d{24, 41}))
                == false;
    });


    auto const slide = suite.test("slide", [](auto check, auto &log) {
        planet::ui::baseplate bp;
        auto r = planet::ui::range{
                planet::debug::fixed_element{log, {50, 10}},
                planet::ui::draggable<planet::debug::fixed_element>{
                        "hs", {log, {10, 10}}},
                {20, 0, 100}};
        r.add_to(bp);
        r.reflow({.screen = screen_constraints}, screen_constraints);
        r.move_to(
                {.screen = screen_constraints},
                {{15, 30}, planet::affine::extents2d{50, 50}});
        r.draw();
        check(r.slider.offset.position()) == planet::affine::point2d{8, 0};
        check(r.slider.offset.width.min()) == 0;
        check(r.slider.offset.width.max()) == 40;
        check(bp.has_focus(r.slider)) == false;

        bp.events.mouse.push(
                {planet::events::button::left,
                 planet::events::action::down,
                 {24, 31}});
        check(bp.has_focus(r.slider)) == true;
        check(r.slider.drag_last.value()) == planet::affine::point2d{24, 31};
        check(r.slider.offset.width.value()) == 8;
        check(r.slider.position())
                == planet::affine::rectangle2d{
                        {23, 30}, planet::affine::extents2d{10, 10}};

        bp.events.mouse.push(
                {planet::events::button::left,
                 planet::events::action::held,
                 {26, 41}});
        check(bp.has_focus(r.slider)) == true;
        check(r.slider.drag_last.value()) == planet::affine::point2d{26, 41};
        check(r.slider.offset.width.value()) == 10;
        check(r.slider.position())
                == planet::affine::rectangle2d{
                        {25, 30}, planet::affine::extents2d{10, 10}};

        bp.events.mouse.push(
                {planet::events::button::left,
                 planet::events::action::held,
                 {36, 31}});
        check(r.slider.drag_last.value()) == planet::affine::point2d{36, 31};
        check(r.slider.offset.width.value()) == 20;
        check(r.slider.position())
                == planet::affine::rectangle2d{
                        {35, 30}, planet::affine::extents2d{10, 10}};

        bp.events.mouse.push(
                {planet::events::button::left,
                 planet::events::action::held,
                 {46, 31}});
        check(r.slider.drag_last.value()) == planet::affine::point2d{46, 31};
        check(r.slider.offset.width.value()) == 30;
        check(r.slider.position())
                == planet::affine::rectangle2d{
                        {45, 30}, planet::affine::extents2d{10, 10}};

        bp.events.mouse.push(
                {planet::events::button::left,
                 planet::events::action::held,
                 {36, 31}});
        check(r.slider.drag_last.value()) == planet::affine::point2d{36, 31};
        check(r.slider.offset.width.value()) == 20;
        check(r.slider.position())
                == planet::affine::rectangle2d{
                        {35, 30}, planet::affine::extents2d{10, 10}};
    });


    auto const knob_z_from_depth =
            suite.test("knob z from depth", [](auto check, auto &log) {
                planet::ui::baseplate bp;
                auto r = planet::ui::range{
                        planet::debug::fixed_element{log, {50, 10}},
                        planet::ui::draggable<planet::debug::fixed_element>{
                                "hs",
                                planet::debug::fixed_element{log, {10, 10}}},
                        {20, 0, 100}};
                r.add_to(bp);
                r.reflow({.screen = screen_constraints}, screen_constraints);
                r.move_to(
                        {.screen = screen_constraints},
                        {{15, 30}, planet::affine::extents2d{50, 50}});
                r.draw();

                /**
                 * The knob is moved within the range's own sub-element move, so
                 * the containment depth lifts it above the range. There is no
                 * longer a manual static bump -- both share the same static z,
                 * and the depth alone is what puts the knob on top.
                 */
                check(r.dynamic_z_layer) == 0.0f;
                check(r.slider.dynamic_z_layer) == 1.0f;
                check(r.slider.static_z_layer) == r.static_z_layer;
                check(r.slider.z_layer()) > r.z_layer();
            });


    auto const knob_above_a_high_screen =
            suite.test("knob above a high screen", [](auto check, auto &log) {
                /**
                 * The pitch editor's slider lives inside a modal, so the
                 * range's static z is raised over the modal screen's before
                 * `add_to` runs -- that is when the knob inherits it. The knob
                 * then rises above the range from the containment depth alone,
                 * so the static only has to clear the screen for both of them
                 * to stay above it rather than have their drags hoovered up.
                 */
                planet::ui::baseplate bp;
                planet::ui::screen modal{100.0f};
                modal.add_to(bp);

                auto r = planet::ui::range{
                        planet::debug::fixed_element{log, {50, 10}},
                        planet::ui::draggable<planet::debug::fixed_element>{
                                "hs",
                                planet::debug::fixed_element{log, {10, 10}}},
                        {20, 0, 100}};
                r.static_z_layer = modal.static_z_layer + 1;
                r.add_to(modal);
                r.reflow({.screen = screen_constraints}, screen_constraints);
                r.move_to(
                        {.screen = screen_constraints},
                        {{15, 30}, planet::affine::extents2d{50, 50}});

                check(r.static_z_layer) == 101.0f;
                check(r.slider.static_z_layer) == 101.0f;
                check(r.z_layer()) > modal.z_layer();
                check(r.slider.z_layer()) > modal.z_layer();
                check(r.slider.z_layer()) > r.z_layer();
            });


}
