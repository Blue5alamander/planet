#include <planet/events/mouse.hpp>
#include <felspar/test.hpp>

#include <felspar/coro/bus.hpp>


namespace {


    auto const suite = felspar::testsuite("mouse");


    auto const clicks = suite.test("clicks", [](auto check) {
        planet::events::mouse_settings config;

        planet::events::mouse e1{}, e2{};
        check(planet::events::is_click(config, e1, e2)) == false;

        e1.button = planet::events::button::left;
        e1.action = planet::events::action::down;
        e2.button = planet::events::button::right;
        e2.action = planet::events::action::up;
        check(planet::events::is_click(config, e1, e2)) == false;

        e2.button = planet::events::button::left;
        check(planet::events::is_click(config, e1, e2)) == true;

        e1.timestamp -= std::chrono::milliseconds{202};
        check(planet::events::is_click(config, e1, e2)) == false;
    });


}
