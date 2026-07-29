#pragma once


#include <planet/affine/point2d.hpp>
#include <planet/events/action.hpp>
#include <planet/events/modifiers.hpp>

#include <felspar/coro/stream.hpp>

#include <chrono>


namespace planet::events {


    enum class button { none, left, right, middle, x1, x2 };
    enum class window_pointer { enter, leave };


    /// ## Low-level mouse clicks and movement
    /**
     * `modifiers` carries the modifier keys held at the time of the event, so
     * consumers can tell a shift-click or a ctrl-drag apart from the plain
     * kind. As with key events, match a combination by comparing against a
     * constructed `events::modifiers` value rather than reading the flags
     * individually.
     */
    struct mouse {
        events::button button = events::button::none;
        events::action action = events::action::released;
        affine::point2d location{{}, {}};
        std::size_t clicks = {};
        events::modifiers modifiers = {};
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::now();
    };


    /// ## Higher level events

    /// ### Mouse button clicks
    struct click final {
        events::button button = events::button::none;
        affine::point2d location = {{}, {}};
        std::size_t count = {};
        events::modifiers modifiers = {};
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::now();
    };


    /// ## Mouse event processing


    /// ### Identifying clicks

    /// #### If a single mouse event is a click
    std::optional<click> is_mouse_click(mouse const &);

    /// #### Convert a stream of mouse events to a stream of clicks
    felspar::coro::stream<click> identify_clicks(felspar::coro::stream<mouse>);


}
