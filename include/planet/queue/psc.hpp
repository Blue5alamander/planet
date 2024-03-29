#pragma once


#include <felspar/coro/coroutine.hpp>
#include <felspar/exceptions/logic_error.hpp>

#include <vector>


namespace planet::queue {


    /// ## Push producer, single consumer queue
    /**
     * A queue connects multiple producers with a single consumer. The consumer
     * must be a coroutine, but the producers may be normal functions. If no
     * coroutine is currently waiting then pushed values are held until one
     * wants a value.
     *
     * The type may be move only.
     *
     * This type cannot be used to create a coroutine, but rather is used to
     * facilitate communication to coroutines.
     *
     * Like with the
     * [bus](https://felspar.com/coro/include/felspar/coro/bus.hpp), it is
     * important that the consumption not kill the producer that just pushed a
     * value.
     */
    template<typename T>
    class psc final {
        std::vector<T> values;
        felspar::coro::coroutine_handle<> waiting = {};

      public:
        using value_type = T;


        psc() = default;
        psc(psc &&) = default;
        psc(psc const &) = delete;

        psc &operator=(psc &&) = default;
        psc &operator=(psc const &) = delete;


        bool empty() const noexcept { return values.empty(); }


        auto next() {
            struct awaitable {
                explicit awaitable(psc &qq) : q{qq} {}
                awaitable(awaitable const &) = delete;
                awaitable(awaitable &&) = delete;
                ~awaitable() {
                    if (mine and q.waiting == mine) { q.waiting = {}; }
                }

                awaitable &operator=(awaitable const &) = delete;
                awaitable &operator=(awaitable &&) = delete;


                psc &q;
                felspar::coro::coroutine_handle<> mine = {};


                bool await_ready() const noexcept {
                    return not q.values.empty();
                }
                void await_suspend(felspar::coro::coroutine_handle<> h) {
                    if (q.waiting) {
                        throw felspar::stdexcept::logic_error{
                                "There is already a coroutine waiting on this "
                                "queue"};
                    } else {
                        mine = h;
                        q.waiting = h;
                    }
                }
                T await_resume() {
                    mine = {};
                    T ret = std::move(q.values.front());
                    q.values.erase(q.values.begin());
                    return ret;
                }
            };
            return awaitable{*this};
        }


        bool push(T t) {
            values.push_back(std::move(t));
            if (waiting) {
                std::exchange(waiting, {}).resume();
                return true;
            } else {
                return false;
            }
        }
    };


    template<>
    class psc<void> final {
        std::size_t pushes = {};
        felspar::coro::coroutine_handle<> waiting = {};


      public:
        using value_type = void;


        psc() = default;
        psc(psc &&) = default;
        psc(psc const &) = delete;

        psc &operator=(psc &&) = default;
        psc &operator=(psc const &) = delete;


        bool empty() const noexcept { return pushes == 0u; }


        auto next() {
            struct awaitable {
                explicit awaitable(psc &qq) : q{qq} {}
                awaitable(awaitable const &) = delete;
                awaitable(awaitable &&) = delete;
                ~awaitable() {
                    if (mine and q.waiting == mine) { q.waiting = {}; }
                }

                awaitable &operator=(awaitable const &) = delete;
                awaitable &operator=(awaitable &&) = delete;


                psc &q;
                felspar::coro::coroutine_handle<> mine = {};


                bool await_ready() const noexcept { return not q.empty(); }
                void await_suspend(felspar::coro::coroutine_handle<> h) {
                    if (q.waiting) {
                        throw felspar::stdexcept::logic_error{
                                "There is already a coroutine waiting on this "
                                "queue"};
                    } else {
                        mine = h;
                        q.waiting = h;
                    }
                }
                void await_resume() {
                    mine = {};
                    --q.pushes;
                }
            };
            return awaitable{*this};
        }


        bool push() {
            ++pushes;
            if (waiting) {
                std::exchange(waiting, {}).resume();
                return true;
            } else {
                return false;
            }
        }
    };


}
