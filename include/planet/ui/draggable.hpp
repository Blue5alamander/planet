#pragma once


#include <planet/events/mouse.hpp>
#include <planet/ui/widget.hpp>


namespace planet::ui {


    /// ## Droppable target
    /**
     * An abstract super class for types that will respond to the "drop" part of
     * the draggable sequence. This happens when the mouse button is released
     * after dragging to a new position.
     */
    struct drop_target {
        using constrained_type = constrained2d<float>;

        /// Respond to the end of a drag
        /**
         * The returned constraint becomes the new offset in the draggable
         * widget. Nearly always this should be zero so that new drags start at
         * the current position rather than random one.
         */
        virtual constrained_type drop(constrained_type const &) = 0;
    };


    /// ## A draggable UI element
    template<typename HotSpot>
    class draggable : public widget {
      public:
        explicit draggable(std::string_view const n, HotSpot hs)
        : widget{n}, hotspot{std::move(hs)} {}


        using constrained_type = typename widget::constrained_type;
        using widget::move_to;


        drop_target *target = nullptr;
        constrained_type offset;
        std::optional<affine::point2d> drag_last;


      protected:
        HotSpot hotspot;


      private:
        constrained_type do_reflow(constrained_type const &constraint) override {
            return hotspot.reflow(constraint);
        }
        void do_move_sub_elements(affine::rectangle2d const &r) override {
            hotspot.move_to({r.top_left, hotspot.constraints().extents()});
        }
        felspar::coro::task<void> behaviour() override {
            auto mouse = widget::events.mouse.values();
            while (true) {
                auto event = co_await mouse.next();
                if (target and event.button == events::button::left) {
                    if (event.action == events::action::down) {
                        drag_last = event.location;
                        widget::baseplate->hard_focus_on(this);
                    } else if (drag_last and event.action == events::action::held) {
                        auto const delta_mouse = event.location - *drag_last;
                        drag_last = event.location;

                        auto const old_offset = offset.position();
                        offset.desire(old_offset + delta_mouse);
                        auto const delta_offset =
                                offset.position() - old_offset;

                        auto const old_position = widget::position();
                        move_to(
                                {old_position.top_left + delta_offset,
                                 old_position.extents});
                    } else if (event.action == events::action::up) {
                        offset = target->drop(offset);
                        drag_last = {};
                        widget::baseplate->hard_focus_off(this);
                    }
                }
            }
        }
    };


}
