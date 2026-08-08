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
     * Screens layer statically. Because a screen covers everything there is
     * nothing to lay one out, so it is added to a baseplate and drawn but
     * never moved, and the containment depth that
     * `reflowable::reflow_parameters` assigns to ordinary widgets never
     * reaches it. Its `dynamic_z_layer` therefore stays at zero and the static
     * value passed to the constructor is the whole of its z layer -- which is
     * why the statics are spread far enough apart to dominate the depths that
     * widget nesting produces.
     *
     * Typically that static will be the lowest one (the default -1) so that
     * events route to any other widget overlaying the screen, and a screen
     * meant to be modal is given one well above every widget it covers.
     *
     * A screen is also a hover boundary, so when screens are stacked only the
     * topmost one receives hover -- those beneath it are cleared as though the
     * pointer had left them.
     *
     * Which events stop at the screen is set by the kinds it swallows. The
     * default is the pointer kinds, so clicks that miss every other widget
     * stop here rather than entering the play area, and key presses carry on
     * down to whatever subscribes below. A modal screen swallows
     * `events::kinds::all()`, leaving the interface it covers inert to the
     * keyboard too.
     */
    class screen final : public widget {
      public:
        screen(float const z = -1,
               events::kinds const s = events::kinds::pointer())
        : widget{"planet::ui::screen", z}, swallowed{s} {
            hover_boundary();
        }
        screen(std::string_view const n,
               float const z = -1,
               events::kinds const s = events::kinds::pointer())
        : widget{n, z}, swallowed{s} {
            hover_boundary();
        }
        screen(screen &&s) : widget{std::move(s)}, swallowed{s.swallowed} {
            if (has_baseplate()) { response.post(behaviour()); }
        }


        using widget::events;
        using widget::panel;


      private:
        events::kinds const swallowed;

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
        felspar::coro::task<void> behaviour() override {
            co_await swallow(swallowed);
        }
    };


}
