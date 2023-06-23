#pragma once


#include <planet/telemetry/performance.hpp>
#include <planet/time/checkpointer.hpp>

#include <atomic>
#include <chrono>


namespace planet::telemetry {


    /// ## Exponential decay
    class exponential_decay final : public performance {
        std::atomic<double> m_value{};
        double decay_rate;

      public:
        static constexpr std::string_view box{"_p:t:exponential_decay"};

        /// ### Construct performance counter
        /**
         * The half-life is expressed in the number of readings that the value
         * will decay over. A small value will be much-smoothed than a larger
         * value, but will also take longer to get to the right level.
         */
        exponential_decay(std::string_view, std::size_t half_life);

        double value() const noexcept;

        void add_measurement(double);

      private:
        bool save(serialise::save_buffer &) override;
    };


    /// ## Real-time exponential decay function
    /**
     * Although this performance counter uses an atomic to store the value, it
     * is not thread safe for anything other than reading of the current value.
     * Updates to the counter must be synchronised if they are to occur from
     * more than one thread.
     */
    class real_time_decay final : public performance {
        std::atomic<double> m_value{};
        time::checkpointer last;
        double const half_life;

      public:
        static constexpr std::string_view box{"_p:t:real_time_decay"};

        real_time_decay(
                std::string_view const n, std::chrono::nanoseconds const hl)
        : performance{n}, half_life{static_cast<double>(hl.count())} {}


        double value() const noexcept;


        void add_measurement(double);

      private:
        bool save(serialise::save_buffer &) override;
    };


    /// ## Time decayed frequency measurement
    /**
     * Although this performance counter uses an atomic to store the value, it
     * is not thread safe for anything other than reading of the current value.
     * Updates to the counter must be synchronised if they are to occur from
     * more than one thread.
     */
    class real_time_rate final : public performance {
        std::atomic<double> m_value{};
        time::checkpointer last;
        double const half_life;

      public:
        static constexpr std::string_view box{"_p:t:real_time_rate"};

        real_time_rate(
                std::string_view const n, std::chrono::nanoseconds const hl)
        : performance{n}, half_life{static_cast<double>(hl.count())} {}
        real_time_rate(
                std::string_view const n,
                std::chrono::nanoseconds const hl,
                double const v)
        : performance{n}, m_value{v}, half_life{static_cast<double>(hl.count())} {}


        double value() const noexcept;


        void tick();


      private:
        bool save(serialise::save_buffer &) override;
    };


}
