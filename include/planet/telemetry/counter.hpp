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


        counter(std::string_view const n) : performance{n} {}
        counter(std::string_view const n, std::int64_t const v)
        : performance{n}, count{v} {}


        std::int64_t value() const noexcept { return count.load(); }


        /// ### Changing the value
        std::int64_t operator++() { return ++count; }
        std::int64_t operator--() { return --count; }
        void operator+=(std::int64_t const c) { count += c; }
        void operator-=(std::int64_t const c) { count -= c; }


        /// #### Set to a specific value
        void value(std::int64_t const v) { count.store(v); }


      private:
        bool save(serialise::save_buffer &) const override;
        bool load(measurements &) override;
    };


}
