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
     *
     * When setting a `parent` `counter` instance, any change to the child
     * will also be applied to the parent.
     */
    class counter final : public performance {
        counter *parent = nullptr;
        std::atomic<std::int64_t> count{};


      public:
        using value_type = std::int64_t;


        static constexpr std::string_view box{"_p:t:counter"};


        counter(std::string_view const n,
                std::source_location const &loc =
                        std::source_location::current())
        : performance{n, loc} {}
        counter(std::string_view const n,
                value_type const v,
                std::source_location const &loc =
                        std::source_location::current())
        : performance{n, loc}, count{v} {}
        counter(std::string_view const n,
                counter &p,
                std::source_location const &loc =
                        std::source_location::current())
        : performance{n, loc}, parent{&p} {}


        value_type value() const noexcept { return count.load(); }


        /// ### Changing the value
        std::int64_t operator++() noexcept {
            if (parent) { ++*parent; }
            return ++count;
        }
        std::int64_t operator--() noexcept {
            if (parent) { --*parent; }
            return --count;
        }
        void operator+=(value_type const c) noexcept {
            if (parent) { *parent += c; }
            count += c;
        }
        void operator-=(value_type const c) noexcept {
            if (parent) { *parent -= c; }
            count -= c;
        }


        /// #### Set to a specific value
        void value(value_type const v) noexcept { count.store(v); }


      private:
        bool save(serialise::save_buffer &) const override;
        bool load(measurements &) override;
    };


}
