#include <planet/events/mouse.hpp>
#include <felspar/test.hpp>

#include <felspar/coro/bus.hpp>


namespace {


    auto const suite = felspar::testsuite("mouse");


    auto const clicks = suite.test("clicks", [](auto check) {
        planet::events::mouse_settings config;

        planet::events::mouse e1{}, e2{};
        check(planet::events::is_click(config, e1, e2)) == false;

        e1.button = planet::events::mouse::press::left;
        e1.pressed = planet::events::mouse::state::down;
        e2.button = planet::events::mouse::press::right;
        e2.pressed = planet::events::mouse::state::up;
        check(planet::events::is_click(config, e1, e2)) == false;

        e2.button = planet::events::mouse::press::left;
        check(planet::events::is_click(config, e1, e2)) == true;

        e1.timestamp -= std::chrono::milliseconds{202};
        check(planet::events::is_click(config, e1, e2)) == false;
    });


}
