#include <planet/events/mouse.hpp>


bool planet::events::is_click(
        mouse_settings const &conf, cursor const &e1, cursor const &e2) {
    return e1.button == e2.button and e1.action == mouse::action::down
            and e2.action == mouse::action::up
            and e2.timestamp - e1.timestamp <= conf.click_time;
}


felspar::coro::stream<planet::events::click> planet::events::identify_clicks(
        mouse_settings const &conf, felspar::coro::stream<cursor> events) {
    std::optional<cursor> down;
    while (auto event = co_await events.next()) {
        if (event->action == mouse::action::down) {
            down = *event;
        } else if (
                event->action == mouse::action::up and down
                and is_click(conf, *down, *event)) {
            co_yield click{
                    event->button, event->location, 1u, event->timestamp};
        } else {
            down = {};
        }
    }
}
