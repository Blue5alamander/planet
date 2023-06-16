#pragma once


#include <chrono>


namespace planet::time {


    /// ## Time checkpoints
    /**
     * Used to calculate time duration between events. A checkpoint is created
     * when the object is first created and then every time one of the timing
     * members is called.
     */
    class checkpointer {
        using clock = std::chrono::steady_clock;
        clock::time_point last = clock::now();

      public:
        /// ### The amount of time since the last check point
        auto time_difference() {
            auto now = std::exchange(last, clock::now());
            return now - last;
        }
        /// ### The number of seconds since the last check point
        auto per_second() {
            return static_cast<double>(time_difference().count())
                    / clock::period::den;
        }
    };


}
