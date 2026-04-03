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
        clock::duration m_duration = {};


      public:
        /// ### Record a new checkpoint
        auto checkpoint() noexcept {
            auto old = std::exchange(last, clock::now());
            m_duration = last - old;
            return m_duration;
        }


        /// ### The time point of the last check point
        clock::time_point last_checkpoint() const noexcept { return last; }


        /// ### The duration between the last two checkpoints
        clock::duration duration() const noexcept { return m_duration; }
    };


}
