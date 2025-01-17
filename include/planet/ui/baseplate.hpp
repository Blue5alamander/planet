#pragma once


#include <planet/events/queue.hpp>
#include <planet/ui/panel.hpp>
#include <planet/ui/forward.hpp>

#include <felspar/coro/starter.hpp>
#include <felspar/memory/stable_vector.hpp>


namespace planet::ui {


    /// ## Base plate for widgets
    /**
     * The `baseplate` manages widget focus and routing of messages to the
     * correct widget.
     */
    class baseplate final {
        friend class widget;
        using widget_ptr = widget *;
        using const_widget_ptr = widget const *;
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
        ~baseplate();

        baseplate &operator=(baseplate const &) = delete;
        /// TODO The move assignment could be allowed
        baseplate &operator=(baseplate &&) = delete;


        /// ### Base panel for local transformations
        panel pixels;


        /// ### Meta-data
        std::size_t widget_count() const noexcept { return widgets.size(); }


        /// ### Event inputs and settings
        events::queue events;
        auto const &last_mouse_event() const noexcept { return last_mouse; }


        /// ### Reset widgets at the start of a frame
        void start_frame_reset();


        /// ### Set and remove hard focus
        void hard_focus_on(widget_ptr const wp) { hard_focus = wp; }
        void hard_focus_off(widget_ptr const w) {
            if (hard_focus == w) { hard_focus = nullptr; }
        }


        /// ### Does this widget have focus
        bool has_focus(const_widget_ptr const wp) const noexcept {
            return wp == find_focused_widget();
        }
        bool has_focus(widget const &wp) const noexcept {
            return has_focus(&wp);
        }


      private:
        /// ### Register and remove widgets from event routing
        void add(widget_ptr);
        void add(widget &w) { add(&w); }
        void remove(widget_ptr const w) {
            if (hard_focus == w) { hard_focus = nullptr; }
            if (soft_focus == w) { soft_focus = nullptr; }
            std::erase(widgets, w);
            std::erase(current_hovers, w);
            std::erase(previous_hovers, w);
        }


        /// ### Hover handling
        events::mouse last_mouse;
        std::vector<widget_ptr> current_hovers;
        std::vector<widget_ptr> previous_hovers;
        std::chrono::steady_clock::time_point last_reset =
                std::chrono::steady_clock::now();

        /// ### Update a widget pointer
        void update_ptr(widget_ptr const was, widget_ptr const now) {
            if (hard_focus == was) { hard_focus = now; }
            if (soft_focus == was) { soft_focus = now; }
            for (widget_ptr &w : widgets) {
                if (w == was) { w = now; }
            }
        }


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
        task_type forward_mouse();
        task_type forward_keys();
        task_type forward_scroll();
    };


}
