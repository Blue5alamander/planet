#pragma once

#include <planet/telemetry/performance.hpp>

#include <atomic>
#include <chrono>


namespace planet::telemetry {


    /// ## Exponentially-decayed duration measurement
    /**
     * Smoothes a wall-clock duration over a configurable number of samples
     * (half-life). Use the nested `measurement` RAII type to time a scope.
     *
     * The stored value is in nanoseconds. Thread-safe for concurrent reads;
     * updates should be serialised by the caller if they occur from multiple
     * threads.
     */
    class steady_duration final : public performance {
        std::atomic<std::int64_t> m_value{};
        double const decay_rate;


      public:
        static constexpr std::string_view box{"_p:t:steady_duration"};


        /// ### Construct
        /**
         * `half_life` is expressed in number of readings over which the value
         * decays by half. A larger value gives smoother but slower-converging
         * measurements.
         */
        steady_duration(
                std::string_view,
                std::size_t half_life,
                std::source_location const & = std::source_location::current());


        /// ### Current smoothed value
        std::chrono::nanoseconds value() const noexcept {
            return std::chrono::nanoseconds{m_value.load()};
        }


        /// ### Record a duration measurement
        void add_measurement(std::chrono::nanoseconds) noexcept;


        /// ### RAII scope timer
        /**
         * Captures the start time on construction and records the elapsed
         * duration into the parent `steady_duration` counter on destruction.
         */
        class measurement final {
            steady_duration &counter;
            std::chrono::steady_clock::time_point const start =
                    std::chrono::steady_clock::now();


          public:
            explicit measurement(steady_duration &c) noexcept : counter{c} {}
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
