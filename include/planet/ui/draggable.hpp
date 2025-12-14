#pragma once


#include <planet/events/mouse.hpp>
#include <planet/ui/widget.hpp>


namespace planet::ui {


    /// ## Droppable target
    /**
     * An abstract super class for types that will respond to the "drop" part of
     * the draggable sequence. This happens when the mouse button is released
     * after dragging to a new position.
     *
     * TODO Probably the `start`/`update` methods should be able to cancel the
     * drag/drop.
     */
    struct drop_target {
        using constrained_type = constrained2d<float>;

        /// ### Start dragging
        virtual void start(constrained_type const &);
        /**
         * The constraint is passed when the mouse button is pressed to start a
         * drag.
         *
         * The default implementation does nothing.
         */

        /// ### Updates during drag
        virtual void update(constrained_type const &);
        /**
         * Passes the constraint every time a mouse message is processed to
         * update the drag position of the slider.
         *
         * The default implementation does nothing.
         */

        /// ### Respond to the end of a drag
        virtual constrained_type drop(constrained_type const &) = 0;
        /**
         * This must be implemented. It is passed the new location that the
         * draggable has been placed at.
         *
         * The returned constraint becomes the new offset in the draggable
         * widget. Nearly always this should be zero so that new drags start at
         * the current position rather than random one.
         */
    };


    /// ## A draggable UI element
    template<typename ColdSpot, typename HotSpot = ColdSpot &>
    class draggable final : public widget {
      public:
        draggable(std::string_view const n, ColdSpot cs)
        : widget{n},
          hoverable{false},
          coldspot{std::move(cs)},
          hotspot{coldspot} {}
        draggable(std::string_view const n, ColdSpot cs, HotSpot hs)
        : widget{n},
          hoverable{true},
          coldspot{std::move(cs)},
          hotspot{std::move(hs)} {}


        using constrained_type = typename widget::constrained_type;
        using reflow_parameters = typename widget::reflow_parameters;
        using widget::move_to;


        drop_target *target = nullptr;
        constrained_type offset;
        std::optional<affine::point2d> drag_last;


      protected:
        /**
         * TODO Tracking what to do with the `hotspot` through `hoverable` is a
         * bit ugly. It's almost certainly better to have a partial template
         * specialisation for the case where `HotSpot` is `void` instead.
         */
        bool hoverable, hovering = false;
        ColdSpot coldspot;
        HotSpot hotspot;

        void do_draw() override {
            if (not hoverable) {
                coldspot.draw();
            } else if (hovering) {
                hotspot.draw();
            } else {
                coldspot.draw();
            }
        }


      private:
        void hover(std::chrono::nanoseconds const ts) override {
            hovering = (ts.count() > 0);
        }
        widget::reflow_parameters reflow_p;
        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &constraint) override {
            reflow_p = p;
            if (not hoverable) {
                return coldspot.reflow(p, constraint);
            } else if (hovering) {
                coldspot.reflow(p, constraint);
                return hotspot.reflow(p, constraint);
            } else {
                hotspot.reflow(p, constraint);
                return coldspot.reflow(p, constraint);
            }
        }
        affine::rectangle2d do_move_sub_elements(
                reflow_parameters const &p,
                affine::rectangle2d const &r) override {
            if (not hoverable) {
                return coldspot.move_to(p, r);
            } else if (hovering) {
                coldspot.move_to(p, r);
                return hotspot.move_to(p, r);
            } else {
                hotspot.move_to(p, r);
                return coldspot.move_to(p, r);
            }
        }
        felspar::coro::task<void> behaviour() override {
            auto mouse = widget::events.mouse.values();
            while (true) {
                auto event = co_await mouse.next();
                if (target and event.button == events::button::left) {
                    if (event.action == events::action::down) {
                        drag_last = event.location;
                        widget::hard_focus_on();
                        target->start(offset);
                    } else if (drag_last and event.action == events::action::held) {
                        auto const delta_mouse = event.location - *drag_last;
                        drag_last = event.location;

                        auto const old_offset = offset.position();
                        offset.desire(old_offset + delta_mouse);
                        auto const delta_offset =
                                offset.position() - old_offset;

                        auto const old_position = widget::position();
                        move_to(reflow_p,
                                {old_position.top_left + delta_offset,
                                 old_position.extents});
                        target->update(offset);
                    } else if (event.action == events::action::up) {
                        offset = target->drop(offset);
                        drag_last = {};
                        widget::hard_focus_off();
                    }
                }
            }
        }
    };


}
