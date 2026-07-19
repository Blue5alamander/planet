#pragma once


#include <planet/affine/extents2d.hpp>

#include <chrono>
#include <optional>


namespace planet::events {


    /// ## Window resize transition
    enum class window_resize { minimise, change, maximise, full_screen };


    /// ## Window resize event
    /**
     * The new `extents` are only carried by a `window_resize::change`; the
     * minimise, maximise and full screen transitions are state changes without
     * a size of their own, so their `extents` are left unset.
     */
    struct resize final {
        events::window_resize transition;
        std::optional<affine::extents2d> extents = {};
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::now();
    };


}
