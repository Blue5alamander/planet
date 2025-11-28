#pragma once


#include <planet/telemetry/performance.hpp>

#include <atomic>
#include <cstdint>


namespace planet::telemetry {


    /// ## A performance counter
    /**
     * The counter is thread safe for all writes and reads.
     *
     * When loaded the saved measurement is added to any currently recorded
     * measurement.
     */
    class counter final : public performance {
        std::atomic<std::int64_t> count{};


      public:
        static constexpr std::string_view box{"_p:t:counter"};


        counter(std::string_view const n,
                std::source_location const &loc =
                        std::source_location::current())
        : performance{n, loc} {}
        counter(std::string_view const n,
                std::int64_t const v,
                std::source_location const &loc =
                        std::source_location::current())
        : performance{n, loc}, count{v} {}


        std::int64_t value() const noexcept { return count.load(); }


        /// ### Changing the value
        std::int64_t operator++() noexcept { return ++count; }
        std::int64_t operator--() noexcept { return --count; }
        void operator+=(std::int64_t const c) noexcept { count += c; }
        void operator-=(std::int64_t const c) noexcept { count -= c; }


        /// #### Set to a specific value
        void value(std::int64_t const v) noexcept { count.store(v); }


      private:
        bool save(serialise::save_buffer &) const override;
        bool load(measurements &) override;
    };


}
