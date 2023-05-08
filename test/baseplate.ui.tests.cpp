#include <planet/debug/ui.hpp>
#include <planet/ui/baseplate.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;


    auto const suite = felspar::testsuite("baseplate");


    auto const ordering = suite.test("ordering", [](auto check, auto &log) {
        planet::ui::panel panel;
        planet::ui::baseplate<std::ostream &> bp;
        planet::debug::button btn;

        check(bp.widget_count()) == 0u;
        check(btn.is_visible()) == false;

        btn.add_to(bp, panel);

        check(bp.widget_count()) == 1u;
        check(btn.is_visible()) == true;

        check([&]() {
            btn.draw(log);
        }).throws(std::logic_error{"Reflowable position has not been set"});

        check([&]() {
            btn.move_to({{15, 20}, planet::affine::extents2d{40, 30}});
        }).throws(std::logic_error{"Reflowable constraints have not been set"});
    });


    auto const events = suite.test("events", [](auto check) {
        planet::ui::baseplate<std::ostream &> bp;
        planet::ui::panel panel;
        planet::debug::button btn;

        btn.add_to(bp, panel);
        btn.reflow({{40, 0, 40}, {30, 0, 30}});
        btn.move_to({{15, 20}, planet::affine::extents2d{4, 3}});
        check(btn.clicks) == 0u;

        bp.events.mouse.push(
                {planet::events::button::left,
                 planet::events::action::down,
                 {18, 21}});
        bp.events.mouse.push(
                {planet::events::button::left,
                 planet::events::action::up,
                 {18, 21}});
        check(btn.clicks) == 1u;

        bp.events.mouse.push(
                {planet::events::button::left,
                 planet::events::action::down,
                 {8, 21}});
        bp.events.mouse.push(
                {planet::events::button::left,
                 planet::events::action::up,
                 {8, 21}});
        check(btn.clicks) == 1u;

        bp.events.mouse.push(
                {planet::events::button::left,
                 planet::events::action::down,
                 {18, 21}});
        bp.events.mouse.push(
                {planet::events::button::left,
                 planet::events::action::up,
                 {18, 21}});
        check(btn.clicks) == 2u;
    });


}
