#include <planet/events/mouse.hpp>


bool planet::events::is_click(
        mouse_settings const &conf, mouse const &e1, mouse const &e2) {
    return e1.button == e2.button and e1.action == action::down
            and e2.action == action::up
            and e2.timestamp - e1.timestamp <= conf.click_time;
}


felspar::coro::stream<planet::events::click> planet::events::identify_clicks(
        mouse_settings const &conf, felspar::coro::stream<mouse> events) {
    std::optional<mouse> down;
    while (auto event = co_await events.next()) {
        if (event->action == action::down) {
            down = *event;
        } else if (
                event->action == action::up and down
                and is_click(conf, *down, *event)) {
            co_yield click{
                    event->button, event->location, 1u, event->timestamp};
        } else {
            down = {};
        }
    }
}
