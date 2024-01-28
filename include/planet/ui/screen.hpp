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
    class screen final : public widget {
      public:
        screen(float const z = -1) : widget{"planet::ui::screen", z} {}
        screen(std::string_view const n, float const z = -1) : widget{n, z} {}


        using constrained_type = typename widget::constrained_type;
        using widget::events;
        using widget::panel;


      private:
        constrained_type do_reflow(constrained_type const &c) override {
            return c;
        }
        affine::rectangle2d
                do_move_sub_elements(affine::rectangle2d const &r) override {
            return r;
        }
        void do_draw() override{};
        bool contains_global_coordinate(
                affine::point2d const &,
                felspar::source_location const &) const override {
            return true;
        }
        bool wants_focus() const noexcept override { return true; }
        felspar::coro::task<void> behaviour() override { co_return; }
    };


}
