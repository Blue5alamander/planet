#pragma once


#include <planet/telemetry/performance.hpp>


namespace planet::telemetry {


    /// ## Maximum number
    /**
     * Captures the largest recorded number. The value starts at zero.
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


    /// ## Minimum number
    /**
     * Captures the smallest recorded number. The value starts at the largest
     * possible number, a sentinel for "nothing recorded yet" that the first
     * recorded value always replaces.
     *
     * The representation is signed, so recorded values may be negative.
     *
     * When setting a `parent` `smin` instance, any value presented to the child
     * will also be presented to the parent.
     */
    class smin final : public performance {
        smin *parent = nullptr;
        static constexpr std::int64_t maximum =
                std::numeric_limits<std::int64_t>::max();
        std::atomic<std::int64_t> m_value{maximum};


      public:
        static constexpr std::string_view box{"_p:t:min"};


        using value_type = std::int64_t;


        smin(std::string_view const n,
             std::source_location const &loc = std::source_location::current())
        : performance{n, loc} {}
        smin(std::string_view const n,
             smin &p,
             std::source_location const &loc = std::source_location::current())
        : performance{n, loc}, parent{&p} {}


        value_type value() const noexcept { return m_value.load(); }
        void value(value_type const v) noexcept;


      private:
        bool save(serialise::save_buffer &) const override;
        bool load(measurements &) override;
    };


}
