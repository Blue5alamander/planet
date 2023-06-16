#pragma once


#include <planet/telemetry/performance.hpp>

#include <atomic>
#include <chrono>


namespace planet::telemetry {


    /// ## Time decayed rate
    class rate final : public performance {
        std::atomic<float> m_value{};
        double const half_life;


      public:
        rate(std::string_view const n, std::chrono::nanoseconds const hl)
        : performance{n}, half_life{static_cast<double>(hl.count())} {}
        rate(std::string_view const n,
             std::chrono::nanoseconds const hl,
             float const v)
        : performance{n}, m_value{v}, half_life{static_cast<double>(hl.count())} {}


        float value() const noexcept { return m_value.load(); }


        void reading(float, std::chrono::nanoseconds);


      private:
        bool save(serialise::save_buffer &) override;
    };


}
