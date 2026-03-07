#pragma once


#include <planet/serialise/base_types.hpp>
#include <planet/serialise/load_buffer.hpp>
#include <planet/serialise/save_buffer.hpp>

#include <chrono>


namespace planet::serialise {


    template<typename Rep, typename Period>
    void save(save_buffer &ab, std::chrono::duration<Rep, Period> const d) {
        ab.save_box_lambda(2, "_sc::duration", [&]() {
            std::int64_t const num = Period::num;
            std::int64_t const den = Period::den;
            auto const count = d.count();
            save(ab, num, den, count);
        });
    }
    template<typename Rep, typename Period>
    void load(box &b, std::chrono::duration<Rep, Period> &d) {
        b.lambda("_sc::duration", [&]() {
            if (b.version == 2) {
                std::int64_t num = {}, den = {};
                Rep count = {};
                b.fields(num, den, count);
                d = std::chrono::duration<Rep, Period>{count};
            } else if (b.version == 1) {
                Rep count;
                b.named("_sc::duration", count);
                d = std::chrono::duration<Rep, Period>{count};
            } else {
                b.throw_unsupported_version(2);
            }
        });
    }


    template<typename Clock, typename Duration>
    void
            save(save_buffer &ab,
                 std::chrono::time_point<Clock, Duration> const &tp) {
        ab.save_box("_sc::time_point", tp.time_since_epoch().count());
    }
    template<typename Clock, typename Duration>
    void load(box &b, std::chrono::time_point<Clock, Duration> &tp) {
        decltype(tp.time_since_epoch().count()) c;
        b.named("_sc::time_point", c);
        tp = std::chrono::time_point<Clock, Duration>{Duration{c}};
    }


}
