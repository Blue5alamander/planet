#pragma once


#include <planet/affine/point2d.hpp>
#include <planet/events/modifiers.hpp>

#include <chrono>


namespace planet::events {


    /// ## Scroll wheel events
    /**
     * `modifiers` carries the modifier keys held at the time of the event, so
     * a consumer can tell a ctrl-scroll (conventionally zoom) apart from a
     * plain one. As with key and mouse events, match a combination by
     * comparing against a constructed `events::modifiers` value rather than
     * reading the flags individually.
     */
    struct scroll final {
        float dx{}, dy{};
        affine::point2d location = {{}, {}};
        events::modifiers modifiers = {};
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::now();
    };


}
