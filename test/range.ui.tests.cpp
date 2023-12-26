#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/range.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("range.ui");


    auto const reflow = suite.test("reflow", [](auto check, auto &log) {
        planet::ui::baseplate bp;
        auto r = planet::debug::printable<planet::ui::range<
                planet::debug::fixed_element,
                planet::debug::printable<
                        planet::ui::draggable<planet::debug::fixed_element>>>>{
                planet::debug::fixed_element{log, {50, 10}},
                planet::debug::printable<
                        planet::ui::draggable<planet::debug::fixed_element>>{
                        "hs", planet::debug::fixed_element{log, {10, 10}}},
                {20, 0, 100}};
        r.add_to(bp);
        r.reflow({{50, 0, 100}, {50, 0, 100}});
        r.move_to({{15, 30}, planet::affine::extents2d{50, 50}});

        check(r.is_visible()) == true;
        check(r.wants_focus()) == true;
        check(r.slider.is_visible()) == true;
        check(r.slider.wants_focus()) == true;

        check(r.position())
                == planet::affine::rectangle2d{
                        {15, 30}, planet::affine::extents2d{50, 10}};

        check(r.contains_global_coordinate({16, 21})) == false;
        check(r.contains_global_coordinate({16, 31})) == true;
        check(r.contains_global_coordinate({16, 41})) == false;

        check(r.slider.offset.min()) == planet::affine::extents2d{0, 0};
        check(r.slider.offset.extents()) == planet::affine::extents2d{0, 0};
        check(r.slider.offset.max()) == planet::affine::extents2d{40, 0};

        check(r.slider.position())
                == planet::affine::rectangle2d{
                        {15, 30}, planet::affine::extents2d{10, 10}};

        check(r.slider.contains_global_coordinate({16, 21})) == false;
        check(r.slider.contains_global_coordinate({16, 31})) == true;
        check(r.slider.contains_global_coordinate({16, 41})) == false;
    });


    auto const slide = suite.test("slide", [](auto check, auto &log) {
        planet::ui::baseplate bp;
        auto r = planet::debug::printable<planet::ui::range<
                planet::debug::fixed_element,
                planet::debug::printable<
                        planet::ui::draggable<planet::debug::fixed_element>>>>{
                planet::debug::fixed_element{log, {50, 10}},
                planet::debug::printable<
                        planet::ui::draggable<planet::debug::fixed_element>>{
                        "hs", {log, {10, 10}}},
                {20, 0, 100}};
        r.add_to(bp);
        r.reflow({{50, 0, 100}, {50, 0, 100}});
        r.move_to({{15, 30}, planet::affine::extents2d{50, 50}});
        check(r.slider.offset.width.min()) == 0;
        check(r.slider.offset.width.max()) == 40;

        bp.events.mouse.push(
                {planet::events::button::left,
                 planet::events::action::down,
                 {16, 31}});
        check(bp.has_focus(r.slider)) == true;
        check(r.slider.drag_last.value()) == planet::affine::point2d{16, 31};
        check(r.slider.offset.width.value()) == 0;
        check(r.slider.position())
                == planet::affine::rectangle2d{
                        {15, 30}, planet::affine::extents2d{10, 10}};

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
