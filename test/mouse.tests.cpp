#include <planet/events/mouse.hpp>
#include <felspar/test.hpp>

#include <felspar/coro/bus.hpp>


namespace {


    auto const suite = felspar::testsuite("mouse");


    auto const clicks = suite.test("clicks", [](auto check) {
        planet::events::mouse_settings config;

        planet::events::mouse e1{}, e2{};
        check(planet::events::is_click(config, e1, e2)) == false;

        e1.which = planet::events::mouse::button::left;
        e1.pressed = planet::events::mouse::state::down;
        e2.which = planet::events::mouse::button::right;
        e2.pressed = planet::events::mouse::state::up;
        check(planet::events::is_click(config, e1, e2)) == false;

        e2.which = planet::events::mouse::button::left;
        check(planet::events::is_click(config, e1, e2)) == true;

        e1.timestamp -= std::chrono::milliseconds{102};
        check(planet::events::is_click(config, e1, e2)) == false;
    });


}
