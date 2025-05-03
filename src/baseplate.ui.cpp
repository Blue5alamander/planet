#include <planet/telemetry/rate.hpp>
#include <planet/ui/baseplate.hpp>
#include <planet/ui/widget.hpp>

#include <planet/log.hpp>


namespace {
    planet::telemetry::exponential_decay c_widget_list_length{
            "planet_ui_baseplate_widget_count", 20};
    planet::telemetry::exponential_decay c_hover_list_length{
            "planet_ui_baseplate_hover_count", 5};
}


planet::ui::baseplate::~baseplate() {
    if (not widgets.empty()) {
        planet::log::warning(
                "Baseplate still has", widgets.size(),
                "widgets attached to it");
        for (auto w : widgets) { w->baseplate = nullptr; }
    }
}


void planet::ui::baseplate::start_frame_reset() {
    c_widget_list_length.add_measurement(widgets.size());

    // Hover handling
    auto const now = std::chrono::steady_clock::now();
    auto const elapsed = now - last_reset;
    last_reset = now;
    for (auto widget : widgets) {
        if (widget->contains_global_coordinate(last_mouse.location)) {
            widget->hover(widget->hover_time += elapsed);
            current_hovers.push_back(widget);
            std::erase(previous_hovers, widget);
        }
    }
    c_hover_list_length.add_measurement(current_hovers.size());
    for (auto widget : previous_hovers) {
        widget->hover(widget->hover_time = {});
    }
    previous_hovers.clear();
    std::swap(previous_hovers, current_hovers);

    // Widgets and focus
    if (soft_focus
        and std::find(widgets.begin(), widgets.end(), soft_focus)
                == widgets.end()) {
        soft_focus = nullptr;
    }
    if (hard_focus
        and std::find(widgets.begin(), widgets.end(), hard_focus)
                == widgets.end()) {
        hard_focus = nullptr;
    }
    widgets.clear();
}


void planet::ui::baseplate::add(widget_ptr const w) {
    w->baseplate = this;
    widgets.push_back(w);
    update_if_better_soft_focus(w);
}


void planet::ui::baseplate::update_if_better_soft_focus(widget_ptr w) {
    if (w->wants_focus()
        and (not soft_focus or soft_focus->z_layer < w->z_layer)
        and w->contains_global_coordinate(last_mouse.location)) {
        soft_focus = w;
    }
}


auto planet::ui::baseplate::forward_mouse() -> task_type {
    try {
        auto mouse = events.mouse.values();
        while (true) {
            last_mouse = co_await mouse.next();
            // Look for the widget that should now have soft focus
            soft_focus = nullptr;
            for (widget_ptr w : widgets) { update_if_better_soft_focus(w); }
            // Now send the event to the correct widget
            if (auto *send_to = find_focused_widget(); send_to) {
                send_to->events.mouse.push(last_mouse);
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
