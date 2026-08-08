#pragma once


#include <planet/events/kinds.hpp>
#include <planet/events/queue.hpp>
#include <planet/ui/panel.hpp>
#include <planet/ui/forward.hpp>

#include <felspar/coro/starter.hpp>
#include <felspar/memory/stable_vector.hpp>

#include <optional>


namespace planet::ui {


    /// ## Base plate for widgets
    /**
     * The `baseplate` manages widget focus and routing of messages to the
     * correct widget.
     */
    class baseplate final {
        /// ### Widget bookkeeping
        friend class widget;
        using widget_ptr = widget *;
        using const_widget_ptr = widget const *;

        /// #### Current frame live list
        std::vector<widget_ptr> widgets;

        /// #### Widgets bound to this baseplate
        std::vector<widget_ptr> attached;
        /**
         * `widgets` is rebuilt every frame — `start_frame_reset` clears it and
         * each widget re-adds itself as it draws — so it only ever holds the
         * widgets drawn in the current frame. `attached` instead persists for
         * as long as a widget is bound to this baseplate through `add_to`. The
         * destructor walks `attached` to null each widget's back-pointer, so a
         * widget that outlives the baseplate (a screen held in a static, say)
         * will not later try to deregister itself through a dangling pointer.
         */


      public:
        /// ### Construction
        baseplate() {
            forwarders.post(forward_mouse());
            forwarders.post(forward_keys());
            forwarders.post(forward_scroll());
            forwarders.post(forward_text());
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

        /// #### The most recent mouse event
        /**
         * Empty when there is no pointer to speak of — before the first mouse
         * event of the run, and after `pointer_left` reports that the pointer
         * has gone. With no pointer location nothing can be hovered and
         * nothing can take the soft focus.
         */
        auto const &last_mouse_event() const noexcept { return last_mouse; }

        /// #### The pointer location, if there is one
        std::optional<affine::point2d> pointer_location() const noexcept {
            if (last_mouse) {
                return last_mouse->location;
            } else {
                return {};
            }
        }

        /// #### The pointer has left
        /**
         * Clears the pointer location so that stale coordinates from wherever
         * the pointer was last seen stop counting as hovers. Widgets hovered
         * up to now have their hover time reset by the next
         * `start_frame_reset`.
         */
        void pointer_left() noexcept { last_mouse = {}; }


        /// ### Reset widgets at the start of a frame
        void start_frame_reset();


        /// ### Set and remove hard focus
        /**
         * The capture says which event kinds are taken out of positional
         * routing and delivered here whatever the pointer is over. Kinds left
         * out of it route as though this widget held no focus at all.
         */
        void hard_focus_on(
                widget_ptr const wp,
                events::kinds const c = events::kinds::all()) {
            hard_focus = wp;
            hard_focus_captures = c;
        }
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


        /// ### Forward an event to the next widget in the stack
        /**
         * Each event is delivered to the topmost widget in the focus stack
         * with a subscriber for that event kind. A widget whose behaviour
         * receives an event it chooses not to act on can pass it on to
         * whatever it covers by handing the event to one of these.
         * Forwarding follows the same rule delivery does, so the event goes to
         * the next subscriber down and stops when the stack runs out. Returns
         * the widget the event was pushed to, or `nullptr` if nothing was left
         * to take it.
         *
         * The stack is consumed by the delivery, so an event must be
         * forwarded while it is being handled: once the next event arrives
         * the stack describes that event instead.
         */
        widget *forward(events::mouse const &);
        widget *forward(events::key const &);
        widget *forward(events::scroll const &);
        widget *forward(events::text const &);


      private:
        /// ### Register and remove widgets from event routing
        void add(widget_ptr);
        void add(widget &w) { add(&w); }
        /// ### Bind a widget for back-pointer cleanup on destruction
        void attach(widget_ptr const w) { attached.push_back(w); }
        void remove(widget_ptr const w) {
            if (hard_focus == w) { hard_focus = nullptr; }
            if (soft_focus == w) { soft_focus = nullptr; }
            std::erase(widgets, w);
            std::erase(current_hovers, w);
            std::erase(previous_hovers, w);
            std::erase(focus_stack, w);
            std::erase(attached, w);
        }


        /// ### Hover handling
        std::optional<events::mouse> last_mouse;
        std::vector<widget_ptr> current_hovers;
        std::vector<widget_ptr> previous_hovers;
        std::vector<widget_ptr> under_pointer;
        /**
         * `under_pointer` is a scratch buffer reused each frame to collect the
         * widgets under the pointer before sorting and walking them, so the
         * allocation is amortised rather than done fresh every frame. It is
         * cleared at the start of `route_hover_events` and holds no state
         * across frames.
         */
        std::chrono::steady_clock::time_point last_reset =
                std::chrono::steady_clock::now();

        /// #### Deliver this frame's hover notifications
        /**
         * Run once per frame from `start_frame_reset` before the live widget
         * list is cleared, so it sees the widgets drawn in the frame that has
         * just finished rather than an empty list.
         *
         * Every widget under the pointer is notified with its accumulated
         * hover duration, and every widget hovered last frame but not this one
         * is notified with a zero duration. That zero is the only signal a
         * widget gets that the pointer has left it.
         *
         * The widgets under the pointer are visited from the highest z layer
         * down, each accumulating hover duration, until a widget marked as a
         * hover boundary (see `widget::hover_boundary`) is reached. That
         * widget is still hovered, but anything stacked beneath it is given a
         * zero duration instead, so a tooltip or other overlay can stop hover
         * from reaching the widgets it covers.
         */
        void route_hover_events();


        /// ### Update a widget pointer
        void update_ptr(widget_ptr const was, widget_ptr const now) {
            if (hard_focus == was) { hard_focus = now; }
            if (soft_focus == was) { soft_focus = now; }
            for (widget_ptr &w : widgets) {
                if (w == was) { w = now; }
            }
            for (widget_ptr &w : attached) {
                if (w == was) { w = now; }
            }
        }


        /// ### Focus handling
        /**
         * Soft focus tracks the topmost focusable widget under the pointer
         * and is what `has_focus` reports in the absence of hard focus. Hard
         * focus is managed by the application and is used by a widget to
         * "capture" events even if the mouse moves away -- it places that
         * widget at the top of the delivery stack.
         */
        widget_ptr soft_focus = nullptr, hard_focus = nullptr;
        /// #### Which kinds the hard focus takes out of positional routing
        events::kinds hard_focus_captures;
        widget_ptr find_focused_widget() const noexcept {
            return hard_focus ? hard_focus : soft_focus;
        }
        /// #### Update the soft focus if this widget is better
        void update_if_better_soft_focus(widget_ptr);

        /// #### Event delivery
        /**
         * Each event is offered down a stack of the focusable widgets under
         * the pointer. The stack is ordered like the widgets appear on the
         * display: the lowest z layer at the front of the array and the
         * highest at the back, with any hard focus widget above everything.
         * An event goes to the first widget with a subscriber for its kind,
         * and there are no exceptions. Widgets are popped off the top of the
         * stack until one is found, so a widget that ignores a kind lets those
         * events fall through to whatever it covers. A widget that does take
         * an event can still pass it on down the stack through the public
         * `forward` overloads.
         *
         * The stack is built for one event kind at a time, because delivering
         * the events consumes the stack..
         */
        std::vector<widget_ptr> focus_stack;
        void build_focus_stack(bool events::kinds::*);
        template<typename Ev>
        widget_ptr
                forward(planet::queue::pmc<Ev> planet::events::queue::*,
                        Ev const &);

        /// #### Give up focus held by a widget that was not drawn
        /**
         * Run once per frame from `start_frame_reset` before the live widget
         * list is cleared. A widget only appears in that list by drawing
         * itself, so one that has stopped being drawn -- a modal that has
         * closed, or a screen that has been navigated away from -- would
         * otherwise go on holding the focus and receiving key presses while
         * invisible.
         *
         * Both kinds of focus are dropped this way, including the hard focus a
         * widget took to capture events, since a widget that is no longer
         * drawn has no way to release the capture itself.
         */
        void drop_stale_focus() noexcept;


        /// ### Event forwarding
        felspar::coro::starter<> forwarders;
        using task_type = typename felspar::coro::starter<>::task_type;
        task_type forward_mouse();
        task_type forward_keys();
        task_type forward_scroll();
        task_type forward_text();
    };


}
