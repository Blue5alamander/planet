#include <planet/events/mouse.hpp>

#include <optional>


felspar::coro::stream<planet::events::click>
        planet::events::identify_clicks(felspar::coro::stream<mouse> events) {
    std::optional<mouse> down;
    while (auto event = co_await events.next()) {
        if (event->action == action::down) {
            down = *event;
        } else if (event->action == action::up and down) {
            co_yield click{
                    event->button, event->location, event->clicks,
                    event->timestamp};
            down = {};
        } else {
            down = {};
        }
    }
}
