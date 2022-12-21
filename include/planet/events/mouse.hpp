#pragma once


#include <planet/affine/point2d.hpp>

#include <felspar/coro/stream.hpp>

#include <chrono>


namespace planet::events {


    /// Configuration for timing and movements metrics
    struct mouse_settings {
        std::chrono::milliseconds click_time{100};
    };


    /// Low-level mouse clicks and movement
    class mouse final {
      public:
        enum class button { none, left, right, middle, x1, x2 };
        enum class state { released, down, held, up };

        button which = button::none;
        state pressed = state::released;
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::now();
        affine::point2d location{{}, {}}, scroll{{}, {}};
    };


    /// Higher level events
    class click final {
      public:
        mouse::button pressed;
        std::chrono::steady_clock::time_point timestamp;
        affine::point2d location;
        std::size_t count;
    };


    /// Given two raw mouse events in time order, is this a click?
    bool is_click(mouse_settings const &, mouse const &, mouse const &);
    /// Process a stream of mouse data into higher level events
    felspar::coro::stream<click> identify_clicks(
            mouse_settings const &, felspar::coro::stream<mouse>);


}
