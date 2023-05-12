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

        drop_target *target = nullptr;
        constrained_type offset;


      protected:
        HotSpot hotspot;


      private:
        constrained_type do_reflow(constrained_type const &constraint) override {
            return hotspot.reflow(constraint);
        }
        void do_move_sub_elements(affine::rectangle2d const &r) override {
            hotspot.move_to(r);
        }
        felspar::coro::task<void> behaviour() override {
            std::optional<affine::point2d> base, start;
            while (true) {
                auto event = co_await superclass::events.mouse.next();
                if (target and event.button == events::button::left) {
                    if (event.action == events::action::down) {
                        start = offset.position();
                        base = superclass::panel.outof(event.location);
                        superclass::baseplate->hard_focus_on(this);
                    } else if (
                            base and start
                            and event.action == events::action::held) {
                        auto const locnow =
                                superclass::panel.outof(event.location);
                        offset.desire(*start + locnow - *base);
                    } else if (event.action == events::action::up) {
                        offset = target->drop(offset);
                        base = {};
                        start = {};
                        superclass::baseplate->hard_focus_off(this);
                    }
                }
            }
        }
    };


}
