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


        using widget::events;
        using widget::panel;


      private:
        constrained_type do_reflow(
                reflow_parameters const &, constrained_type const &c) override {
            return c;
        }
        affine::rectangle2d do_move_sub_elements(
                reflow_parameters const &,
                affine::rectangle2d const &r) override {
            return r;
        }
        void do_draw() override {};
        /**
         * As the catch-all layer the screen contains every location, including
         * the absence of one. That way it still takes the soft focus while the
         * pointer is outside the window, or before it has been seen at all, and
         * key presses continue to route here rather than being dropped.
         */
        bool contains_global_coordinate(
                std::optional<affine::point2d> const &,
                std::source_location const &) const override {
            return true;
        }
        bool wants_focus() const noexcept override { return true; }
        felspar::coro::task<void> behaviour() override { co_return; }
    };


}
