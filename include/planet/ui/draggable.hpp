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
    template<typename Renderer, typename HotSpot>
    class draggable : public widget<Renderer> {
        using superclass = widget<Renderer>;

      public:
        explicit draggable(std::string_view const n, HotSpot hs)
        : superclass{n}, hotspot{std::move(hs)} {}

        using constrained_type = typename widget<Renderer>::constrained_type;
        using superclass::move_to;

        drop_target *target = nullptr;
        constrained_type offset;
        std::optional<affine::point2d> drag_base, drag_last, drag_start;


      protected:
        HotSpot hotspot;


      private:
        constrained_type do_reflow(constrained_type const &constraint) override {
            return hotspot.reflow(constraint);
        }
        void do_move_sub_elements(affine::rectangle2d const &r) override {
            hotspot.move_to(
                    {r.top_left + offset.position(),
                     hotspot.constraints().extents()});
        }
        felspar::coro::task<void> behaviour() override {
            while (true) {
                auto event = co_await superclass::events.mouse.next();
                if (target and event.button == events::button::left) {
                    if (event.action == events::action::down) {
                        drag_start = offset.position();
                        drag_base = event.location;
                        drag_last = event.location;
                        superclass::baseplate->hard_focus_on(this);
                    } else if (
                            drag_base and drag_last and drag_start
                            and event.action == events::action::held) {
                        offset.desire(
                                *drag_start + event.location - *drag_base);
                        auto const r = superclass::position();
                        move_to(
                                {r.top_left + event.location - *drag_last,
                                 r.extents});
                        drag_last = event.location;
                    } else if (event.action == events::action::up) {
                        offset = target->drop(offset);
                        drag_base = {};
                        drag_start = {};
                        drag_last = {};
                        superclass::baseplate->hard_focus_off(this);
                    }
                }
            }
        }
    };


}
