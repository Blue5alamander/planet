#include <planet/events/mouse.hpp>


auto planet::events::is_mouse_click(mouse const &event)
        -> std::optional<click> {
    if (event.action == action::up and event.clicks) {
        return click{
                event.button, event.location, event.clicks, event.modifiers,
                event.timestamp};
    } else {
        return {};
    }
}


felspar::coro::stream<planet::events::click>
        planet::events::identify_clicks(felspar::coro::stream<mouse> events) {
    while (auto event = co_await events.next()) {
        if (auto const click = is_mouse_click(*event); click) {
            co_yield *click;
        }
    }
}
