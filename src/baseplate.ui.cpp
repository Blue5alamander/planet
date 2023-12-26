#include <planet/ui/baseplate.hpp>
#include <planet/ui/widget.hpp>

#include <planet/log.hpp>


void planet::ui::baseplate::add(widget_ptr const w) {
    w->baseplate = this;
    // TODO Ideally the widget isn't already in the set of widgets
    widgets.push_back(w);
}


auto planet::ui::baseplate::forward_mouse() -> task_type {
    try {
        auto mouse = events.mouse.values();
        while (true) {
            auto const m = co_await mouse.next();
            /// Look for the widget that should now have soft focus
            soft_focus = nullptr;
            for (widget_ptr w : widgets) {
                if (w->wants_focus()
                    and (not soft_focus or soft_focus->z_layer < w->z_layer)
                    and w->contains_global_coordinate(m.location)) {
                    soft_focus = w;
                }
            }
            /// Now send the event to the correct widget
            if (auto *send_to = find_focused_widget(); send_to) {
                send_to->events.mouse.push(m);
            }
        }
    } catch (std::exception const &e) {
        log::critical("Baseplate mouse forwarding exception", e.what());
    }
}
auto planet::ui::baseplate::forward_keys() -> task_type {
    try {
        auto key = events.key.values();
        while (true) {
            auto const k = co_await key.next();
            if (auto *send_to = find_focused_widget(); send_to) {
                send_to->events.key.push(k);
            }
        }
    } catch (std::exception const &e) {
        log::critical("Baseplate key forwarding exception", e.what());
    }
}
auto planet::ui::baseplate::forward_scroll() -> task_type {
    try {
        auto scroll = events.scroll.values();
        while (true) {
            auto const s = co_await scroll.next();
            if (auto *send_to = find_focused_widget(); send_to) {
                send_to->events.scroll.push(s);
            }
        }
    } catch (std::exception const &e) {
        log::critical("Baseplate scroll forwarding exception", e.what());
    }
}
