#pragma once


#include <planet/telemetry/performance.hpp>


namespace planet::telemetry {


    /// ## Maximum number
    /**
     * Captures the largest recorded number. The value starts at the smallest
     * possible (negative) number.
     *
     * When setting a `parent` `max` instance, any value presented to the child
     * will also be presented to the parent.
     */
    class max final : public performance {
        max *parent = nullptr;
        static constexpr std::uint64_t minimum =
                std::numeric_limits<std::uint64_t>::min();
        std::atomic<std::uint64_t> m_value{minimum};


      public:
        static constexpr std::string_view box{"_p:t:max"};


        using value_type = std::uint64_t;


        max(std::string_view const n,
            std::source_location const &loc = std::source_location::current())
        : performance{n, loc} {}
        max(std::string_view const n,
            max &p,
            std::source_location const &loc = std::source_location::current())
        : performance{n, loc}, parent{&p} {}


        value_type value() const noexcept { return m_value.load(); }
        void value(value_type const v) noexcept;


      private:
        bool save(serialise::save_buffer &) const override;
        bool load(measurements &) override;
    };


    /// TODO `min` type


}
