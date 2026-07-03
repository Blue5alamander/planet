#pragma once

#include <planet/telemetry/performance.hpp>

#include <atomic>
#include <chrono>


namespace planet::telemetry {


    /// ## Wall-clock thread duty cycle
    /**
     * Measures the fraction of elapsed real time (0 to 1) that a thread spends
     * inside an instrumented scope, exponentially smoothed over a wall-clock
     * half-life. Use the nested `measurement` RAII type to time a scope.
     *
     * Updates are lock-free and allocation-free, so they are safe from
     * real-time threads such as audio callbacks. Reading and saving the
     * current value is safe from any thread, but measurements must not
     * overlap: use one `thread_load` per scope per thread.
     *
     * When measurements stop arriving the value freezes at its last level
     * (like `real_time_rate`). The half-life is never saved, so changing it
     * between runs only affects how quickly a loaded value converges.
     */
    class thread_load final : public performance {
        std::atomic<double> m_value{};
        std::atomic<std::chrono::steady_clock::time_point> last_end{
                std::chrono::steady_clock::now()};
        static_assert(
                decltype(last_end)::is_always_lock_free,
                "The end-of-measurement time point must be lock free so that measurements can be recorded from a real-time thread without taking a lock");
        double const half_life;


      public:
        static constexpr std::string_view box{"_p:t:thread_load"};


        /// ### Construct
        thread_load(
                std::string_view const n,
                std::chrono::nanoseconds const hl,
                std::source_location const &loc =
                        std::source_location::current())
        /**
         * The half-life is the wall-clock duration over which the smoothed
         * load decays by half. A larger value gives smoother but
         * slower-converging measurements.
         */
        : performance{n, loc}, half_life{static_cast<double>(hl.count())} {}


        /// ### Current smoothed duty cycle (0 to 1)
        double value() const noexcept { return m_value.load(); }


        /// ### Record a busy period

        /// #### Record a busy period that ended now
        void add_measurement(std::chrono::nanoseconds busy) noexcept;

        /// #### Record a busy period within a known wall-clock window
        void add_measurement(
                std::chrono::nanoseconds busy,
                std::chrono::nanoseconds elapsed) noexcept;
        /**
         * The deterministic core used by the overload above and by tests. The
         * busy time is blended in as a fraction of the elapsed wall-clock
         * time, weighted by how much of the half-life the window covers.
         */


        /// ### RAII scope measurement
        /**
         * Captures the start time on construction and records the busy period
         * into the parent `thread_load` counter on destruction.
         */
        class measurement final {
            thread_load &counter;
            std::chrono::steady_clock::time_point const start =
                    std::chrono::steady_clock::now();


          public:
            explicit measurement(thread_load &c) noexcept : counter{c} {}
            ~measurement() {
                counter.add_measurement(
                        std::chrono::steady_clock::now() - start);
            }

            measurement(measurement const &) = delete;
            measurement(measurement &&) = delete;
            measurement &operator=(measurement const &) = delete;
            measurement &operator=(measurement &&) = delete;
        };


      private:
        bool save(serialise::save_buffer &) const override;
        bool load(measurements &) override;
    };


}
