#pragma once


namespace planet::time {


    /// ## Timing information
    class time_limiter {
        std::size_t frame_number = {};
        std::chrono::nanoseconds frame_time;
        std::chrono::steady_clock::time_point base_time =
                std::chrono::steady_clock::now();

      public:
        /// ### Construction
        time_limiter(std::chrono::nanoseconds);

        /// ### Calculate the current wait time
        /**
         * Call this to calculate how long the thread should sleep for in order
         * to preserve the requested frame duration.
         *
         * It is possible that this will return a negative time period.
         */
        std::chrono::nanoseconds wait_time();
    };


}
