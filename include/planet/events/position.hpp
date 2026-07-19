#pragma once


#include <planet/affine/point2d.hpp>

#include <chrono>


namespace planet::events {


    /// ## Window position event
    struct position final {
        affine::point2d location;
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::now();
    };


}
