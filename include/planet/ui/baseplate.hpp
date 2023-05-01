#pragma once


#include <planet/ui/widget.hpp>

#include <felspar/coro/start.hpp>


namespace planet::ui {


    /// ## Base plate for widgets
    /**
     * The `baseplate` manages widget focus and routing of messages to the
     * correct widget.
     */
    template<typename Renderer>
    class baseplate final {
        struct widget_ final {
            widget<Renderer> *ptr = nullptr;
            float z_layer = {};
        };
        std::vector<widget_> widgets;
        /// ### Focus handling
        /**
         * Soft focus is handled purely by widget the the mouse is over. In the
         * absence of hard focus all of the forwarded events will go to the
         * widget pointed to by the soft focus (if any). Hard focus is managed
         * by the application and overrides the soft focus. It can be used by a
         * widget to "capture" events even if the mouse moves away.
         */
        widget_ *soft_focus = nullptr, *hard_focus = nullptr;
        widget_ *find_focused_widget() const noexcept {
            return hard_focus ? hard_focus : soft_focus;
        }

      public:
        baseplate() {
            forwarders.post(*this, &baseplate::forward_mouse);
            forwarders.post(*this, &baseplate::forward_keys);
            forwarders.post(*this, &baseplate::forward_scroll);
        }
        baseplate(baseplate const &) = delete;
        baseplate(baseplate &&) = delete;

        baseplate &operator=(baseplate const &) = delete;
        baseplate &operator=(baseplate &&) = delete;

        /// ### Event inputs and settings
        events::mouse_settings mouse_settings;
        events::bus events;

        /// ### Register and remove widgets from event routing
        void add(widget<Renderer> *const w, float const z = {}) {
            w->baseplate = this;
            widgets.push_back({w, z});
        }
        void add(widget<Renderer> &w, float const z = {}) { add(&w, z); }
        void remove(widget<Renderer> *const w) {
            if (hard_focus and hard_focus->ptr == w) { hard_focus = nullptr; }
            if (soft_focus and soft_focus->ptr == w) { soft_focus = nullptr; }
            std::erase_if(widgets, [&](auto const &i) { return i.ptr == w; });
        }

        /// Set and remove hard focus
        void hard_focus_on(widget<Renderer> *const wp) {
            for (auto &w : widgets) {
                if (w.ptr == wp) {
                    hard_focus = &w;
                    return;
                }
            }
        }
        void hard_focus_off(widget<Renderer> *const w) {
            if (hard_focus->ptr == w) { hard_focus = nullptr; }
        }

        /// ### Does this widget have focus
        bool has_focus(widget<Renderer> const *const wp) const noexcept {
            auto const *f = find_focused_widget();
            return f and f->ptr == wp;
        }

      private:
        felspar::coro::starter<> forwarders;
        felspar::coro::starter<>::task_type forward_mouse() {
            while (true) {
                auto const m = co_await events.mouse.next();
                /// Look for the widget that should now have soft focus
                soft_focus = nullptr;
                for (auto &w : widgets) {
                    if (w.ptr->wants_focus()
                        and (not soft_focus or soft_focus->z_layer < w.z_layer)
                        and w.ptr->is_within(m.location)) {
                        soft_focus = &w;
                    }
                }
                /// Now send the event to the correct widget
                if (auto *send_to = find_focused_widget(); send_to) {
                    send_to->ptr->events.mouse.push(m);
                }
            }
        }
        felspar::coro::starter<>::task_type forward_keys() {
            while (true) {
                auto const k = co_await events.key.next();
                if (auto *send_to = find_focused_widget(); send_to) {
                    send_to->ptr->events.key.push(k);
                }
            }
        }
        felspar::coro::starter<>::task_type forward_scroll() {
            while (true) {
                auto const s = co_await events.scroll.next();
                if (auto *send_to = find_focused_widget(); send_to) {
                    send_to->ptr->events.scroll.push(s);
                }
            }
        }
    };


}


template<typename Renderer>
inline void planet::ui::widget<Renderer>::deregister(
        ui::baseplate<Renderer> *const bp, widget *const w) {
    if (bp) { bp->remove(w); }
}


template<typename Renderer>
inline void planet::ui::widget<Renderer>::add_to(
        ui::baseplate<Renderer> &bp, ui::panel &parent, float const z_layer) {
    parent.add_child(panel);
    bp.add(this, z_layer);
    visible = true;
    response.post(*this, &widget::behaviour);
}
