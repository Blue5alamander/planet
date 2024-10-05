#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/range.hpp>
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

        check(r.contains_global_coordinate({16, 21})) == false;
        check(r.contains_global_coordinate({16, 31})) == true;
        check(r.contains_global_coordinate({16, 41})) == false;

        check(r.slider.offset.min_extents()) == planet::affine::extents2d{0, 0};
        check(r.slider.offset.extents()) == planet::affine::extents2d{8, 0};
        check(r.slider.offset.max_extents())
                == planet::affine::extents2d{40, 0};

        check(r.slider.position())
                == planet::affine::rectangle2d{
                        {23, 30}, planet::affine::extents2d{10, 10}};

        check(r.slider.contains_global_coordinate({24, 21})) == false;
        check(r.slider.contains_global_coordinate({24, 31})) == true;
        check(r.slider.contains_global_coordinate({24, 41})) == false;
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


}
