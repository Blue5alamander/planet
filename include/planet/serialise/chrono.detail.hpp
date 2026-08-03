#pragma once


#include <planet/serialise/load_buffer.hpp>

#include <chrono>
#include <numeric>


namespace planet::serialise::detail {


    /// ## Machinery for `planet/serialise/chrono.hpp`
    /**
     * The `save` and `load` overloads themselves are in
     * `planet/serialise/chrono.hpp`, together with the account of why a
     * `std::chrono` value has to be handled the way it is. What is here is the
     * parts of that they are built out of, plus the way in for the readings
     * `load` refuses.
     */


    /// ### Re-count a saved duration into the units it is loaded into
    /**
     * The count in the file is in whatever units it was saved in, which need
     * not be the units it is being loaded into. The two are the same in the
     * usual case of a field saved and loaded as the one type, but a field whose
     * units have been changed since it was last saved, or one counted by a
     * `std::chrono::system_clock` — whose units are the standard library's to
     * choose, and differ between them — will not match, so what is saved
     * alongside the count is the ratio it is counted in and the load scales by
     * it.
     *
     * The scaling is `count * num * Period::den / (den * Period::num)`, which
     * is reduced before it is used and then applied to the whole part of the
     * division before the remainder, so a count the loaded duration can hold is
     * arrived at without the arithmetic on the way running over what it can
     * hold. It truncates towards zero, so it lands where a
     * `std::chrono::duration_cast` of the same pair of units would.
     */
    template<typename Rep, typename Period>
    std::chrono::duration<Rep, Period> recount(
            std::int64_t const num, std::int64_t const den, Rep const count) {
        auto constexpr period_num = static_cast<std::int64_t>(Period::num);
        auto constexpr period_den = static_cast<std::int64_t>(Period::den);
        auto const by_num = std::gcd(num, period_num);
        auto const by_den = std::gcd(period_den, den);
        auto const numerator = (num / by_num) * (period_den / by_den);
        auto const denominator = (den / by_den) * (period_num / by_num);
        if constexpr (std::is_integral_v<Rep>) {
            auto const c = static_cast<std::int64_t>(count);
            auto const whole = (c / denominator) * numerator;
            auto const part = (c % denominator) * numerator / denominator;
            return std::chrono::duration<Rep, Period>{
                    static_cast<Rep>(whole + part)};
        } else {
            return std::chrono::duration<Rep, Period>{
                    count * static_cast<Rep>(numerator)
                    / static_cast<Rep>(denominator)};
        }
    }


    /// ### The count as it stands, whatever its clock counts from
    template<typename Clock, typename Duration>
    void load_since_epoch(box &b, std::chrono::time_point<Clock, Duration> &tp) {
        b.lambda("_sc::time_point", [&]() {
            if (b.version == 2) {
                /**
                 * The time since the clock's epoch is saved as a duration, so
                 * the units it was counted in ride along with the count and the
                 * time point comes back at the same instant (modulo some
                 * possible rounding depending on the units the standard
                 * libraries use) no matter what the clock counts in at either
                 * end.
                 */
                Duration since_epoch;
                b.fields(since_epoch);
                tp = std::chrono::time_point<Clock, Duration>{since_epoch};
            } else if (b.version == 1) {
                typename Duration::rep c;
                b.fields(c);
                tp = std::chrono::time_point<Clock, Duration>{Duration{c}};
            } else {
                b.throw_unsupported_version(2);
            }
        });
    }
    /**
     * Paying no attention to the epoch is also what makes this the way in for
     * the readings `load` refuses. A `std::chrono::steady_clock` count is only
     * of use against another count the same run wrote, so reaching in here for
     * one is saying that the file carries whatever else is needed to make sense
     * of it — as the header of a `planet::log` file written before the log kept
     * its times as offsets does. Nothing writes such a file any more, so there
     * is no saving counterpart to reach for.
     */


}
