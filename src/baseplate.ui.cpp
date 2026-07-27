#include <planet/telemetry/rate.hpp>
#include <planet/ui/baseplate.hpp>
#include <planet/ui/widget.hpp>

#include <planet/log.hpp>

#include <algorithm>


/// ## `planet::ui::baseplate`


namespace {
    planet::telemetry::exponential_decay c_widget_list_length{
            "planet_ui_baseplate_widget_count", 20};
    planet::telemetry::exponential_decay c_hover_list_length{
            "planet_ui_baseplate_hover_count", 5};
}


planet::ui::baseplate::~baseplate() {
    if (not widgets.empty()) {
        planet::log::warning(
                "Baseplate destroyed mid-frame with", widgets.size(),
                "widgets still drawn this frame");
    }
    /**
     * Sever the back-pointer of every widget still bound to this baseplate.
     * `attached` (unlike the per-frame `widgets` list) outlives drawing, so a
     * widget that outlives the baseplate — such as a screen held in a static —
     * is left with a null `baseplate` and will not deregister through a
     * dangling pointer when it is finally destroyed.
     */
    for (auto w : attached) { w->baseplate = nullptr; }
}


/// #### Registering widgets


void planet::ui::baseplate::add(widget_ptr const w) {
    w->baseplate = this;
    widgets.push_back(w);
    update_if_better_soft_focus(w);
}


/// #### Focus re-computation


void planet::ui::baseplate::update_if_better_soft_focus(widget_ptr w) {
    if (w->wants_focus()
        and (not soft_focus or soft_focus->z_layer() < w->z_layer())
        and w->contains_global_coordinate(pointer_location())) {
        soft_focus = w;
    }
}


/// #### Event delivery


void planet::ui::baseplate::build_focus_stack() {
    focus_stack.clear();
    auto const location = pointer_location();
    for (auto w : widgets) {
        if (w != hard_focus and w->wants_focus()
            and w->contains_global_coordinate(location)) {
            focus_stack.push_back(w);
        }
    }
    std::stable_sort(
            focus_stack.begin(), focus_stack.end(),
            [](widget_ptr const l, widget_ptr const r) noexcept {
                return l->z_layer() < r->z_layer();
            });
    if (hard_focus) { focus_stack.push_back(hard_focus); }
}


namespace {
    /**
     * A hover boundary is a pointer barrier, so it consumes the
     * pointer-positioned event kinds whether or not anything subscribes, but
     * keyboard events pass through it to any subscriber beneath.
     */
    template<typename Ev>
    constexpr bool blocked_by_boundary = true;
    template<>
    constexpr bool blocked_by_boundary<planet::events::key> = false;
}
template<typename Ev>
auto planet::ui::baseplate::forward(
        planet::queue::pmc<Ev> planet::events::queue::*const kind, Ev const &ev)
        -> widget_ptr {
    while (not focus_stack.empty()) {
        auto *const w = focus_stack.back();
        focus_stack.pop_back();
        bool const boundary =
                blocked_by_boundary<Ev> and w->is_hover_boundary();
        /**
         * A boundary is the last widget a pointer event can reach, so the
         * rest of the stack is dropped before the push -- the widget can
         * call `forward` while handling the event, and it must not be able
         * to send it past the boundary.
         */
        if (boundary) { focus_stack.clear(); }
        auto &queue = w->events.*kind;
        if (queue.consumer_count() or boundary) {
            queue.push(ev);
            return w;
        }
    }
    return nullptr;
}


auto planet::ui::baseplate::forward(events::mouse const &m) -> widget * {
    return forward(&events::queue::mouse, m);
}
auto planet::ui::baseplate::forward(events::key const &k) -> widget * {
    return forward(&events::queue::key, k);
}
auto planet::ui::baseplate::forward(events::scroll const &s) -> widget * {
    return forward(&events::queue::scroll, s);
}


/// #### Per-frame calculations


void planet::ui::baseplate::start_frame_reset() {
    c_widget_list_length.add_measurement(widgets.size());
    route_hover_events();
    drop_stale_focus();
    widgets.clear();
}


void planet::ui::baseplate::route_hover_events() {
    auto const now = std::chrono::steady_clock::now();
    auto const elapsed = now - last_reset;
    last_reset = now;
    /**
     * Hovering needs a real pointer, so unlike the soft focus this does not
     * pass the empty location down to the widgets: a catch-all layer claims
     * every location and would otherwise count as hovered forever. With no
     * pointer `current_hovers` is left empty and everything hovered last frame
     * has its hover time reset below.
     */
    if (last_mouse) {
        /**
         * Collect everything under the pointer, then walk it from the highest
         * z layer down. Each widget is hovered until a widget marked as a
         * hover boundary is reached -- that one is still hovered, but it
         * blocks hover from passing through to anything stacked beneath it, so
         * those get a hover-clear instead. The clear idiom is the same one
         * used on `previous_hovers` below.
         */
        under_pointer.clear();
        for (auto widget : widgets) {
            if (widget->contains_global_coordinate(last_mouse->location)) {
                under_pointer.push_back(widget);
            }
        }
        std::stable_sort(
                under_pointer.begin(), under_pointer.end(),
                [](widget_ptr const l, widget_ptr const r) noexcept {
                    return l->z_layer() > r->z_layer();
                });
        bool blocked = false;
        for (auto widget : under_pointer) {
            if (blocked) {
                widget->hover(widget->hover_time = {});
                std::erase(previous_hovers, widget);
            } else {
                widget->hover(widget->hover_time += elapsed);
                current_hovers.push_back(widget);
                std::erase(previous_hovers, widget);
                if (widget->is_hover_boundary()) { blocked = true; }
            }
        }
    }
    c_hover_list_length.add_measurement(current_hovers.size());
    for (auto widget : previous_hovers) {
        widget->hover(widget->hover_time = {});
    }
    previous_hovers.clear();
    std::swap(previous_hovers, current_hovers);
}


void planet::ui::baseplate::drop_stale_focus() noexcept {
    auto const was_drawn = [this](widget_ptr const w) {
        return std::find(widgets.begin(), widgets.end(), w) != widgets.end();
    };
    if (soft_focus and not was_drawn(soft_focus)) { soft_focus = nullptr; }
    if (hard_focus and not was_drawn(hard_focus)) { hard_focus = nullptr; }
}


/// #### Event routing


auto planet::ui::baseplate::forward_mouse() -> task_type {
    try {
        auto mouse = events.mouse.values();
        while (true) {
            last_mouse = co_await mouse.next();
            /**
             * The soft focus only feeds `has_focus` now that delivery walks
             * the stack, but it still has to track the pointer.
             */
            soft_focus = nullptr;
            for (widget_ptr w : widgets) { update_if_better_soft_focus(w); }
            build_focus_stack();
            forward(*last_mouse);
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
            build_focus_stack();
            if (auto *const send_to = forward(k); send_to) {
                planet::log::debug(
                        "Sending key press", static_cast<int>(k.scancode),
                        "to widget", send_to->name());
            } else {
                planet::log::debug(
                        "No widget consumed key press",
                        static_cast<int>(k.scancode));
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
            build_focus_stack();
            forward(s);
        }
    } catch (std::exception const &e) {
        log::critical("Baseplate scroll forwarding exception", e.what());
    }
}
