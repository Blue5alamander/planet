#pragma once


#include <planet/affine/point2d.hpp>

#include <chrono>


namespace planet::events {


    /// ## Scroll wheel events
    struct scroll final {
        float dx{}, dy{};
        affine::point2d location = {{}, {}};
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::now();
    };


}
