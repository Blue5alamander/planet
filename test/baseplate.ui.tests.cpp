#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
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


    auto const events = suite.test(
            "events",
            [](auto check) {
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
            },
            [](auto check, auto &log) {
                planet::ui::baseplate<std::ostream &> bp;
                planet::ui::panel panel1, panel2;
                planet::debug::button btn;

                panel1.add_child(panel2);
                panel2.move_to({{6, 7}, planet::affine::extents2d{8, 10}});
                btn.add_to(bp, panel2);
                btn.reflow({{40, 0, 40}, {30, 0, 30}});
                btn.move_to({{15, 20}, planet::affine::extents2d{4, 3}});

                check(btn.is_visible()) == true;
                check(btn.wants_focus()) == true;

                /**
                 * `btn` is now at (15, 20) in `panel2`'s coordinate space,
                 * which is at (6, 7) in `panel1`'s coordinate space. This puts
                 * `btn` at (21, 27) in `panel1`'s coordinate space.
                 */
                log << "btn position: " << btn.position() << '\n';

                check(btn.is_within({10, 14})) == false;
                bp.events.mouse.push(
                        {planet::events::button::left,
                         planet::events::action::down,
                         {10, 14}});
                bp.events.mouse.push(
                        {planet::events::button::left,
                         planet::events::action::up,
                         {10, 14}});
                /// A hit here means that the button has moved in the wrong
                /// direction
                check(btn.clicks) == 0u;

                check(btn.is_within({16, 21})) == false;
                bp.events.mouse.push(
                        {planet::events::button::left,
                         planet::events::action::down,
                         {16, 21}});
                bp.events.mouse.push(
                        {planet::events::button::left,
                         planet::events::action::up,
                         {16, 21}});
                /// A hit here means that the button hasn't moved
                check(btn.clicks) == 0u;

                check(btn.is_within({22, 29})) == true;
                bp.events.mouse.push(
                        {planet::events::button::left,
                         planet::events::action::down,
                         {22, 29}});
                bp.events.mouse.push(
                        {planet::events::button::left,
                         planet::events::action::up,
                         {22, 29}});
                /// A hit here is what we want
                check(btn.clicks) == 1u;
            });


}
