#pragma once


#include <chrono>


namespace planet::events {


    /// ## Window focus change
    enum class window_focus { lost, gained };


    /// ## Window focus event
    struct focus final {
        window_focus change;
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::now();
    };


}
