#pragma once


#include <planet/serialise/forward.hpp>

#include <felspar/coro/task.hpp>

#include <chrono>
#include <vector>


namespace planet::time {


    /// ## Clock
    class clock {
      public:
        using rep = std::uint64_t;
        using period = std::micro;
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
            void await_suspend(felspar::coro::coroutine_handle<>);
            void await_resume() { continuation = {}; }

          private:
            time::clock &clock;
            time::clock::time_point wake_up_time;
            felspar::coro::coroutine_handle<> continuation;
        };
        awaitable sleep(duration);


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
            felspar::coro::coroutine_handle<> continuation;
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


}
