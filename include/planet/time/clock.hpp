#pragma once


#include <planet/serialise/forward.hpp>

#include <felspar/coro/task.hpp>

#include <chrono>
#include <vector>


namespace planet::time {


    /// ## Clock
    /**
     * This clock is intended for game time, and as such the time must be
     * advanced by the game code. This means the clock is completely isolated
     * from the actual passage of time in the real world meaning it can be
     * paused or run faster as the game requires.
     *
     * Time is moved forwards using one of the `advance_` functions. Game code
     * can also wait for a given time or duration using the `wake_at` or `sleep`
     * functions.
     */
    class clock {
      public:
        using rep = std::uint64_t;
        using period = std::nano;
        using duration = std::chrono::duration<rep, period>;
        using time_point = std::chrono::time_point<clock, duration>;


        /// ### Construct a new clock starting at time zero
        clock();


        /// ### Return the current time
        time_point now() const noexcept { return time; }


        /// ### Have a coroutine wait for a given time
        struct awaitable {
            awaitable(time::clock &c, time::clock::time_point tp)
            : clock{c}, wake_up_time{tp} {}
            ~awaitable();

            awaitable(awaitable const &) = delete;
            awaitable(awaitable &&);

            awaitable &operator=(awaitable const &) = delete;
            awaitable &operator=(awaitable &&) = delete;

            bool await_ready() const noexcept { return false; }
            void await_suspend(std::coroutine_handle<>);
            void await_resume() { continuation = {}; }

          private:
            time::clock &clock;
            time::clock::time_point wake_up_time;
            std::coroutine_handle<> continuation;
        };
        awaitable sleep(duration);
        awaitable wake_at(time_point);


        /// ### Advance time
        /**
         * Advance the clock to the next event so long as it occurs before
         * `latest`. Returns the amount of time elapsed before the event. If
         * there is no event before `latest` then zero is returned
         */
        duration advance_one(time_point latest);
        /**
         * Advance the clock to the requested time. Returns the number of events
         * that were continued
         */
        std::size_t advance_by(duration const d) {
            return advance_to(now() + d);
        }
        std::size_t advance_to(time_point);


      private:
        time_point time = {};

        struct event {
            time_point when;
            std::coroutine_handle<> continuation;
            friend bool operator<(event const &e, time_point const tp) {
                return e.when < tp;
            }
            friend bool operator<(time_point const tp, event const &e) {
                return tp < e.when;
            }
            friend bool operator==(event const &, event const &) = default;
        };
        std::vector<event> time_line;


        /// ### Serialisation
        friend void save(serialise::save_buffer &, clock const &);
        friend void load(serialise::box &, clock &);
    };
    void save(serialise::save_buffer &, clock const &);
    void load(serialise::box &, clock &);


    /// ## Proportion of time passed
    /**
     * Proportions may be more than one if the duration is longer than expected.
     */
    inline double proportion(clock::duration const d, clock::duration outof)
    /**
     * Pass in the current duration as the first parameter, and the total
     * duration as the second. Returns the proportion of time passed.
     */
    {
        return static_cast<double>(d.count())
                / static_cast<double>(outof.count());
    }

    inline double
            proportion(clock::time_point const current, clock::duration outof)
    /**
     * Pass the current time as the first parameter and the duration (since time
     * zero) as the second parameter. Returns the propertion of time remaining.
     */
    {
        return proportion(current - clock::time_point{}, outof);
    }
    inline double proportion(
            clock::time_point const current, clock::time_point const ends)
    /**
     * Pass the current time as the first parameter and the end time as the
     * second parameter. Returns the proportion of the total time elapsed. This
     * function assumes a clock starting at zero (like the above game time clock
     * does).
     */
    {
        return proportion(
                current - clock::time_point{}, ends - clock::time_point{});
    }


}
