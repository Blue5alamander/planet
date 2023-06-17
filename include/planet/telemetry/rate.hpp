#pragma once


#include <planet/telemetry/performance.hpp>
#include <planet/time/checkpointer.hpp>

#include <atomic>
#include <chrono>


namespace planet::telemetry {


    /// ## Time decayed rate
    class real_time_rate final : public performance {
        std::atomic<float> m_value{};
        time::checkpointer last;
        double const half_life;

        float calculate_decay();

      public:
        real_time_rate(
                std::string_view const n, std::chrono::nanoseconds const hl)
        : performance{n}, half_life{static_cast<double>(hl.count())} {}
        real_time_rate(
                std::string_view const n,
                std::chrono::nanoseconds const hl,
                float const v)
        : performance{n}, m_value{v}, half_life{static_cast<double>(hl.count())} {}


        float value() const noexcept;


        void tick();


      private:
        bool save(serialise::save_buffer &) override;
    };


}
