#pragma once


#include <planet/log.hpp>
#include <planet/ui/widget.hpp>

#include <felspar/coro/starter.hpp>
#include <felspar/memory/stable_vector.hpp>


namespace planet::ui {


    /// ## Base plate for widgets
    /**
     * The `baseplate` manages widget focus and routing of messages to the
     * correct widget.
     */
    template<typename Renderer>
    class baseplate final {
        using widget_ptr = widget<Renderer> *;
        std::vector<widget_ptr> widgets;


      public:
        /// ### Construction
        baseplate() {
            forwarders.post(forward_mouse());
            forwarders.post(forward_keys());
            forwarders.post(forward_scroll());
        }
        baseplate(baseplate const &) = delete;
        baseplate(baseplate &&) = delete;

        baseplate &operator=(baseplate const &) = delete;
        baseplate &operator=(baseplate &&) = delete;


        /// ### Base panel for local transformations
        panel pixels;


        /// ### Meta-data
        std::size_t widget_count() const noexcept { return widgets.size(); }


        /// ### Event inputs and settings
        events::mouse_settings mouse_settings;
        events::queue events;


        /// ### Register and remove widgets from event routing
        void add(widget<Renderer> *const w) {
            w->baseplate = this;
            /// TODO Ideally the widget isn't already in the set of widgets
            widgets.push_back(w);
        }
        void add(widget<Renderer> &w) { add(&w); }
        void remove(widget<Renderer> *const w) {
            if (hard_focus == w) { hard_focus = nullptr; }
            if (soft_focus == w) { soft_focus = nullptr; }
            std::erase(widgets, w);
        }


        /// ### Set and remove hard focus
        void hard_focus_on(widget<Renderer> *const wp) {
            /// TODO Check that wp is already in the set of widgets
            hard_focus = wp;
        }
        void hard_focus_off(widget<Renderer> *const w) {
            if (hard_focus == w) { hard_focus = nullptr; }
        }


        /// ### Does this widget have focus
        bool has_focus(widget<Renderer> const *const wp) const noexcept {
            return wp == find_focused_widget();
        }
        bool has_focus(widget<Renderer> const &wp) const noexcept {
            return has_focus(&wp);
        }


      private:
        /// ### Focus handling
        /**
         * Soft focus is handled purely by widget the the mouse is over. In the
         * absence of hard focus all of the forwarded events will go to the
         * widget pointed to by the soft focus (if any). Hard focus is managed
         * by the application and overrides the soft focus. It can be used by a
         * widget to "capture" events even if the mouse moves away.
         */
        widget_ptr soft_focus = nullptr, hard_focus = nullptr;
        widget_ptr find_focused_widget() const noexcept {
            return hard_focus ? hard_focus : soft_focus;
        }

        /// ### Event forwarding
        felspar::coro::starter<> forwarders;
        using task_type = typename felspar::coro::starter<>::task_type;
        task_type forward_mouse() {
            try {
                auto mouse = events.mouse.values();
                while (true) {
                    auto const m = co_await mouse.next();
                    /// Look for the widget that should now have soft focus
                    soft_focus = nullptr;
                    for (widget_ptr w : widgets) {
                        if (w->wants_focus()
                            and (not soft_focus
                                 or soft_focus->z_layer < w->z_layer)
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
        task_type forward_keys() {
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
        task_type forward_scroll() {
            try {
                auto scroll = events.scroll.values();
                while (true) {
                    auto const s = co_await scroll.next();
                    if (auto *send_to = find_focused_widget(); send_to) {
                        send_to->events.scroll.push(s);
                    }
                }
            } catch (std::exception const &e) {
                log::critical(
                        "Baseplate scroll forwarding exception", e.what());
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
        ui::baseplate<Renderer> &bp, ui::panel &parent) {
    parent.add_child(panel);
    bp.add(this);
    visible = true;
    response.post(*this, &widget::behaviour);
}
