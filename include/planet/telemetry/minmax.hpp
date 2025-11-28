#pragma once


#include <planet/telemetry/performance.hpp>


namespace planet::telemetry {


    /// ## Maximum number
    /**
     * Captures the largest recorded number. The value starts at the smallest
     * possible (negative) number.
     */
    class max final : public performance {
        static constexpr std::uint64_t minimum =
                std::numeric_limits<std::uint64_t>::min();
        std::atomic<std::uint64_t> m_value{minimum};


      public:
        static constexpr std::string_view box{"_p:t:max"};


        using value_type = std::uint64_t;


        max(std::string_view const n,
            std::source_location const &loc = std::source_location::current())
        : performance{n, loc} {}


        value_type value() const noexcept { return m_value.load(); }
        void value(value_type const v) noexcept;


      private:
        bool save(serialise::save_buffer &) const override;
        bool load(measurements &) override;
    };


    /// TODO `min` type


}
