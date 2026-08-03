#pragma once


#include <planet/serialise/base_types.hpp>
#include <planet/serialise/load_buffer.hpp>
#include <planet/serialise/save_buffer.hpp>

#include <chrono>
#include <numeric>


namespace planet::serialise {


    /// ## Time points and time differences
    /**
     * A `std::chrono` value keeps almost nothing about itself at run time. The
     * count is the whole of the value: the units it is counted in are the
     * `Period` of its type and the point it is counted from is the `Clock` of
     * its type, neither of which exists anywhere in memory to be written out.
     * A count saved on its own therefore says only "this many of something",
     * and it is the type named at the other end that decides what it turns
     * back into — which is right while the two ends agree, and silently wrong
     * the moment they don't.
     *
     * They can disagree in two ways that are nobody's mistake. A field's units
     * get changed — milliseconds to samples, say — while files written before
     * the change are still around to be loaded. And `std::chrono::system_clock`
     * counts in units the standard library picks rather than the code:
     * libstdc++ counts nanoseconds where libc++ counts microseconds, so the one
     * `system_clock::time_point` field is a thousandfold out between two builds
     * of the same source, and a project file written by one of them is read
     * wrongly by the other.
     *
     * So the ratio the count is in is saved beside the count, and the load
     * scales from the ratio it finds in the file to the units it has been asked
     * to load into (`detail::recount`). A time point is saved as the duration
     * since its clock's epoch, so it is carried by the same machinery and lands
     * at the instant it was saved at whatever either end counts in.
     *
     * What that leaves a reader able to say about a loaded value:
     *
     * * The length of time is exact, in the units loaded into, truncated
     *   towards zero. Loading into coarser units than were saved drops the tail
     *   of the value, never its magnitude.
     * * The epoch is **not** in the file. The clock comes from the type at the
     *   loading end and nothing checks it against the one that saved: a
     *   `steady_clock` reading is a distance from an arbitrary point in the
     *   run that wrote it and means nothing in any other run, where a
     *   `system_clock` reading is a wall clock instant that keeps its meaning.
     *   Which of those a value is has to be settled where the field is
     *   declared, because the file cannot settle it later.
     * * A box from before the ratio was saved has only its count, so it can
     *   only be taken to already be in the units being loaded into.
     */


    namespace detail {
        /// ### Re-count a saved duration into the units it is loaded into
        /**
         * The count in the file is in whatever units it was saved in, which
         * need not be the units it is being loaded into. The two are the same
         * in the usual case of a field saved and loaded as the one type, but a
         * field whose units have been changed since it was last saved, or one
         * counted by a `std::chrono::system_clock` — whose units are the
         * standard library's to choose, and differ between them — will not
         * match, so what is saved alongside the count is the ratio it is
         * counted in and the load scales by it.
         *
         * The scaling is `count * num * Period::den / (den * Period::num)`,
         * which is reduced before it is used and then applied to the whole part
         * of the division before the remainder, so a count the loaded duration
         * can hold is arrived at without the arithmetic on the way running over
         * what it can hold. It truncates towards zero, so it lands where a
         * `std::chrono::duration_cast` of the same pair of units would.
         */
        template<typename Rep, typename Period>
        std::chrono::duration<Rep, Period>
                recount(std::int64_t const num,
                        std::int64_t const den,
                        Rep const count) {
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
    }


    /// ### Time duration
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
                d = detail::recount<Rep, Period>(num, den, count);
            } else if (b.version == 1) {
                /**
                 * The units aren't in this version, so this could be just
                 * garbage. Load it anyway, and whatever code is using it will
                 * just have to deal with the problem.
                 *
                 * The alternative would be to throw, but it's likely that these
                 * times are informational rather than part of any logic so
                 * hopefully anybody looking at them can handle the weirdness
                 * well enough.
                 */
                Rep count;
                b.named("_sc::duration", count);
                d = std::chrono::duration<Rep, Period>{count};
            } else {
                b.throw_unsupported_version(2);
            }
        });
    }


    /// ### Time points
    template<typename Clock, typename Duration>
    void
            save(save_buffer &ab,
                 std::chrono::time_point<Clock, Duration> const &tp) {
        ab.save_box(2, "_sc::time_point", tp.time_since_epoch());
    }
    template<typename Clock, typename Duration>
    void load(box &b, std::chrono::time_point<Clock, Duration> &tp) {
        b.lambda("_sc::time_point", [&]() {
            if (b.version == 2) {
                /**
                 * The time since the clock's epoch is saved as a duration, so
                 * the units it was counted in ride along with the count and the
                 * time point comes back at the same instant (modulo some possible
                 * rounding depending on the units the standard libraries use)
                 * no matter what the clock counts in at either end.
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


}
