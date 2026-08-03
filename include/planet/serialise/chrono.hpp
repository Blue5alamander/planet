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
     * The epoch a time point counts from is the other half of the same
     * problem, and it cannot be written into the file at all — an epoch is a
     * moment, not a number, and the only thing a clock can say about its own is
     * where it happens to be now. So the count goes in as it stands, and
     * whether it still means anything at the far end is settled by the clock
     * the field was declared with:
     *
     * * `std::chrono::system_clock` counts from a fixed moment in history, so
     *   its reading is a wall clock instant and keeps its meaning wherever it
     *   is read.
     * * A game clock such as `planet::time::clock` counts from the start of
     *   play, which stays put in the same way for the game it belongs to — play
     *   time doesn't run on while the game is paused or while a save sits
     *   unopened — so its reading comes back the game time it went in as.
     * * `std::chrono::steady_clock` counts from an arbitrary moment in the run
     *   that read it, usually when the machine booted, so its reading means
     *   nothing in any other run. Saving one is refused: the `save` and `load`
     *   for a steady time point are deleted, and `planet::log` is where steady
     *   times belong. It resolves them against the start of the run at the
     *   point of capture — where the epoch is still known — and only the offset
     *   reaches the file.
     *
     * What that leaves a reader able to say about a loaded value:
     *
     * * The length of time is exact, in the units loaded into, truncated
     *   towards zero. Loading into coarser units than were saved drops the tail
     *   of the value, never its magnitude.
     * * A time point is at the instant it was saved at, as told by the clock it
     *   is declared with. Nothing checks that clock against the one that saved
     *   the value, because the file cannot settle it later — which is why it
     *   has to be settled where the field is declared.
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
                 std::chrono::time_point<Clock, Duration> const tp) {
        ab.save_box(2, "_sc::time_point", tp.time_since_epoch());
    }
    namespace detail {
        /// #### The count as it stands, whatever its clock counts from
        template<typename Clock, typename Duration>
        void load_since_epoch(
                box &b, std::chrono::time_point<Clock, Duration> &tp) {
            b.lambda("_sc::time_point", [&]() {
                if (b.version == 2) {
                    /**
                     * The time since the clock's epoch is saved as a duration,
                     * so the units it was counted in ride along with the count
                     * and the time point comes back at the same instant (modulo
                     * some possible rounding depending on the units the
                     * standard libraries use) no matter what the clock counts
                     * in at either end.
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
         * Paying no attention to the epoch is also what makes this the way in
         * for the readings `load` refuses. A `std::chrono::steady_clock` count
         * is only of use against another count the same run wrote, so reaching
         * in here for one is saying that the file carries whatever else is
         * needed to make sense of it — as the header of a `planet::log` file
         * written before the log kept its times as offsets does. Nothing writes
         * such a file any more, so there is no saving counterpart to reach for.
         */
    }

    template<typename Clock, typename Duration>
    void load(box &b, std::chrono::time_point<Clock, Duration> &tp) {
        detail::load_since_epoch(b, tp);
    }

    /// #### `std::chrono::steady_clock`
    template<typename Duration>
    void
            save(save_buffer &,
                 std::chrono::time_point<std::chrono::steady_clock, Duration>) =
                    delete;
    template<typename Duration>
    void load(
            box &,
            std::chrono::time_point<std::chrono::steady_clock, Duration> &) =
            delete;
    /**
     * A `std::chrono::steady_clock` reading is a distance from a moment only
     * the run that took it knows, so written into a file it says nothing that
     * can be read back. Log it instead —
     * `planet::log::info("arrived", when)` — and the log resolves it to an
     * offset from the start of the run while the epoch is still there to
     * resolve it against.
     *
     * A file of your own that has to hold steady times needs the same shape:
     * pick one reading for the rest of the file to be measured against, write
     * the wall clock instant it was taken at, and save every other time as a
     * `std::chrono::duration` from it.
     */


}
