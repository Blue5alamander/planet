#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/baseplate.hpp>
#include <planet/ui/layout.column.hpp>
#include <planet/ui/layout.row.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;

    constexpr planet::ui::reflowable::constrained_type screen{
            {40, 0, 40}, {30, 0, 30}};


    auto const suite = felspar::testsuite("baseplate");


    auto const ordering = suite.test("ordering", [](auto check, auto &log) {
        planet::ui::panel panel;
        planet::ui::baseplate bp;
        planet::debug::button<> btn{log};

        check(bp.widget_count()) == 0u;

        btn.add_to(bp, panel);

        check(bp.widget_count()) == 0u;

        check([&]() {
            btn.draw();
        }).throws(std::logic_error{"Reflowable position has not been set"});

        check([&]() {
            btn.move_to(
                    {.screen = screen},
                    {{15, 20}, planet::affine::extents2d{40, 30}});
        }).throws(std::logic_error{"Reflowable constraints have not been set"});
    });


    auto const events = suite.test(
            "events",
            [](auto check, auto &log) {
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                planet::debug::button<> btn{log};

                btn.add_to(bp, panel);
                btn.reflow({.screen = screen}, screen);
                btn.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{4, 3}});
                btn.draw();
                check(btn.clicks) == 0u;

                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::down,
                         .location = {18, 21},
                         .clicks = 0});
                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::up,
                         .location = {18, 21},
                         .clicks = 1});
                check(btn.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
                check(btn.clicks) == 1u;

                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::down,
                         .location = {8, 21}});
                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::up,
                         .location = {8, 21}});
                check(btn.clicks) == 1u;

                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::down,
                         .location = {18, 21}});
                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::up,
                         .location = {18, 21},
                         .clicks = 1});
                check(btn.clicks) == 2u;
            },
            [](auto check, auto &log) {
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                auto ui = planet::ui::column{std::tuple{
                        planet::ui::row{std::tuple{planet::debug::button{
                                planet::debug::fixed_element{log, {4, 3}}}}}}};

                auto &btn = std::get<0>(std::get<0>(ui.items).items);
                btn.add_to(bp, panel);
                ui.reflow({.screen = screen}, screen);
                ui.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{4, 3}});
                ui.draw();

                log << "btn position: " << btn.position() << '\n';

                check(btn.wants_focus()) == true;

                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::down,
                         .location = {16, 21}});
                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::up,
                         .location = {16, 21},
                         .clicks = 1});
                check(btn.clicks) == 1u;
            });


}
