#include <planet/log.hpp>
#include <planet/telemetry/duration.hpp>
#include <planet/time/clock.hpp>
#include <planet/time/format.hpp>
#include <planet/time/rate-limiter.hpp>

#include <planet/serialise/chrono.hpp>

#include <algorithm>


/// ## `planet::time`


std::string planet::time::display_string(std::chrono::nanoseconds const d) {
    auto const ns = d.count();
    auto const abs_ns = ns < 0 ? -ns : ns;
    auto const sign = ns < 0 ? std::string{"-"} : std::string{};

    if (abs_ns <= 999) {
        /// Below a microsecond the nanosecond count is shown verbatim
        return std::to_string(ns) + "ns";
    } else if (abs_ns < 100'000'000'000) {
        /// Up to 100s: the value rounded to three significant figures
        /**
         * Reduce the magnitude to a three digit mantissa in `[100, 999]`,
         * tracking the power of ten dropped (`scale`) and the total digit
         * count so the decimal point and unit can be placed afterwards.
         */
        std::int64_t scale = 1, mantissa = abs_ns;
        int digits = 3;
        while (mantissa >= 1'000) {
            mantissa /= 10;
            scale *= 10;
            ++digits;
        }
        mantissa = (abs_ns + scale / 2) / scale;
        if (mantissa >= 1'000) {
            /// Rounding carried into a fourth digit, so step up a decade
            mantissa = 100;
            ++digits;
        }

        int unit_exponent;
        char const *unit;
        if (digits <= 6) {
            unit_exponent = 3;
            unit = "µs";
        } else if (digits <= 9) {
            unit_exponent = 6;
            unit = "ms";
        } else {
            unit_exponent = 9;
            unit = "s";
        }

        int const integer_digits = digits - unit_exponent;
        auto const mstr = std::to_string(mantissa);
        auto value = mstr.substr(0, integer_digits);
        if (integer_digits < 3) { value += "." + mstr.substr(integer_digits); }
        return sign + value + unit;
    } else {
        /// From 100s on: `1d2h3m45s`, with days unbounded
        auto const total_s = (abs_ns + 500'000'000) / 1'000'000'000;
        auto const days = total_s / 86'400;
        auto rest = total_s % 86'400;
        auto const hours = rest / 3'600;
        rest %= 3'600;
        auto const minutes = rest / 60;
        auto const seconds = rest % 60;

        auto out = sign;
        if (days) { out += std::to_string(days) + "d"; }
        if (days or hours) { out += std::to_string(hours) + "h"; }
        if (days or hours or minutes) { out += std::to_string(minutes) + "m"; }
        return out + std::to_string(seconds) + "s";
    }
}


void planet::time::save(serialise::save_buffer &ab, clock const &c) {
    ab.save_box(clock::box, c.time);
}
void planet::time::load(serialise::box &box, clock &c) {
    box.named(clock::box, c.time);
}


/// ## `planet::time::clock`


planet::time::clock::clock() {}


auto planet::time::clock::sleep(duration const d) -> awaitable {
    return {*this, now() + d};
}


auto planet::time::clock::wake_at(time_point const tp) -> awaitable {
    return {*this, tp};
}


auto planet::time::clock::advance_one(time_point const latest) -> duration {
    if (not time_line.empty() and time_line.front().when <= latest) {
        auto const d = time_line.front().when - time;
        advance_to(time_line.front().when);
        return d;
    } else {
        return {};
    }
}


std::size_t planet::time::clock::advance_to(time_point const latest) {
    std::size_t run{};
    while (not time_line.empty() and time_line.front().when <= latest) {
        time = time_line.front().when;
        time_line.front().continuation.resume();
        time_line.erase(time_line.begin());
        ++run;
    }
    time = latest;
    return run;
}


/// ## `planet::time::clock::awaitable`


planet::time::clock::awaitable::awaitable(awaitable &&a)
: clock{a.clock},
  wake_up_time{a.wake_up_time},
  continuation{std::exchange(a.continuation, {})} {}


planet::time::clock::awaitable::~awaitable() {
    if (continuation) {
        std::erase(
                clock.time_line,
                time::clock::event{wake_up_time, continuation});
    }
}


void planet::time::clock::awaitable::await_suspend(
        std::coroutine_handle<> const h) {
    continuation = h;
    auto const pos = std::upper_bound(
            clock.time_line.begin(), clock.time_line.end(), wake_up_time);
    clock.time_line.insert(pos, {wake_up_time, h});
}


// ## `planet::time::time_limiter`


planet::time::time_limiter::time_limiter(std::chrono::nanoseconds const ns)
: frame_time{ns} {}


std::chrono::nanoseconds planet::time::time_limiter::wait_time() {
    ++frame_number;
    auto const wait_until = base_time + frame_time * frame_number;
    return wait_until - std::chrono::steady_clock::now();
}


// ## `planet::serialise` log formatters for chrono


namespace {
    inline void format_duration_from_ratio(
            std::ostream &os,
            std::int64_t const num,
            std::int64_t const den,
            auto const count) {
        auto const ns = count * (1000'000'000 * num / den);
        os << planet::time::display_string(
                std::chrono::nanoseconds{static_cast<std::int64_t>(ns)});
    }

    auto const duration_print = planet::log::format(
            "_sc::duration", [](std::ostream &os, planet::serialise::box &box) {
                if (box.version == 2) {
                    std::int64_t num, den;
                    box.fields(num, den);
                    // Check marker for count field to determine signedness
                    auto const marker = box.content.extract_marker();
                    if (marker == planet::serialise::marker::i64le) {
                        auto const count = box.content.extract<std::int64_t>();
                        format_duration_from_ratio(os, num, den, count);
                    } else if (marker == planet::serialise::marker::u64le) {
                        auto const count = box.content.extract<std::uint64_t>();
                        format_duration_from_ratio(os, num, den, count);
                    } else {
                        os << "[unknown duration rep type " << to_string(marker)
                           << ']';
                    }
                } else {
                    auto const marker = box.content.extract_marker();
                    if (marker == planet::serialise::marker::i64le) {
                        auto const count = box.content.extract<std::int64_t>();
                        format_duration_from_ratio(os, 1, 1'000'000'000, count);
                    } else {
                        os << "[unknown duration rep type" << to_string(marker)
                           << ']';
                    }
                }
            });

    auto const game_clock_print = planet::log::format(
            planet::time::clock::box,
            [](std::ostream &os, planet::serialise::box &box) {
                planet::time::clock c;
                load(box, c);
                os << "[game time +";
                format_duration_from_ratio(
                        os, planet::time::clock::duration::period::num,
                        planet::time::clock::duration::period::den,
                        c.now().time_since_epoch().count());
                os << ']';
            });

    auto const steady_duration_print = planet::log::format(
            planet::telemetry::steady_duration::box,
            [](std::ostream &os, planet::serialise::box &box) {
                std::string name;
                std::int64_t ns;
                box.named(planet::telemetry::steady_duration::box, name, ns);
                os << name << " = ";
                format_duration_from_ratio(os, 1, 1'000'000'000, ns);
            });

    /**
     * libc++ is a buggy mess with the system_clock. It gets the magnitude
     * completely wrong, so we can't do this yet.
     */
    // auto const time_point_print = planet::log::format(
    //         "_sc::time_point",
    //         [](std::ostream &os, planet::serialise::box &box) {
    //             std::chrono::system_clock::duration::rep d;
    //             box.named("_sc::time_point", d);
    //             auto const time = std::chrono::system_clock::time_point{
    //                     std::chrono::system_clock::duration{d}};
    //             os << time;
    //         });
}
