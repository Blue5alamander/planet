#pragma once


#include <planet/affine/point2d.hpp>
#include <planet/events/action.hpp>

#include <felspar/coro/stream.hpp>

#include <chrono>


namespace planet::events {


    /// ## Configuration for timing and movements metrics
    struct mouse_settings {
        std::chrono::milliseconds click_time{200};
    };


    enum class button { none, left, right, middle, x1, x2 };


    /// ## Low-level mouse clicks and movement
    struct mouse {
        events::button button = events::button::none;
        events::action action = events::action::released;
        affine::point2d location{{}, {}}, scroll{{}, {}};
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::now();
    };


    /// ## Higher level events

    /// ### Mouse button clicks
    struct click final {
        events::button button = events::button::none;
        affine::point2d location = {{}, {}};
        std::size_t count = {};
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::now();
    };


    /// ## Mouse event processing

    /// ### Given two raw mouse events in time order, is this a click?
    bool is_click(mouse_settings const &, mouse const &, mouse const &);
    /// ### Process a stream of mouse data into higher level events
    felspar::coro::stream<click> identify_clicks(
            mouse_settings const &, felspar::coro::stream<mouse>);


}
