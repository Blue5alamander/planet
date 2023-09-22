#pragma once


#include <planet/ui/widget.hpp>


namespace planet::ui {


    /// ## Whole screen UI widget
    /**
     * This widget is infinitely large so will hoover up any events that aren't
     * captured by other widgets. Unless an interface is comprised only of
     * buttons then a screen widget can be used to capture clicks that would
     * enter the play area.
     *
     * Typically the `z_layer` used for a screen widget will be the lowest one
     * to ensure that events are routed to any other widget that overlays the
     * screen.
     */
    template<typename Renderer>
    class screen final : public widget<Renderer> {
        using superclass = widget<Renderer>;

      public:
        screen() : widget<Renderer>{"planet::ui::screen"} {}
        screen(std::string_view const n) : widget<Renderer>{n} {}

        using constrained_type = typename superclass::constrained_type;
        using superclass::events;
        using superclass::panel;

      private:
        constrained_type do_reflow(constrained_type const &c) override {
            return c;
        }
        void do_move_sub_elements(affine::rectangle2d const &) override {}
        void do_draw(Renderer &) override{};
        bool contains_global_coordinate(
                affine::point2d const &,
                felspar::source_location const &) const override {
            return true;
        }
        bool wants_focus() const override { return true; }
        felspar::coro::task<void> behaviour() override { co_return; }
    };


}
