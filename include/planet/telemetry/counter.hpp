#pragma once


#include <planet/telemetry/performance.hpp>

#include <atomic>


namespace planet::telemetry {


    /// ## A performance counter
    class counter final : public performance {
        std::atomic<std::int64_t> count{};


      public:
        counter(std::string_view const n) : performance{n} {}
        counter(std::string_view const n, std::int64_t const v)
        : performance{n}, count{v} {}


        std::int64_t value() const noexcept { return count.load(); }


        std::int64_t operator++() { return ++count; }
        std::int64_t operator--() { return --count; }
        void value(std::int64_t const v) { return count.store(v); }


      private:
        bool save(serialise::save_buffer &) override;
    };


}
