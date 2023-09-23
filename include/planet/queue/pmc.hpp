#pragma once


#include <felspar/coro/coroutine.hpp>
#include <felspar/coro/stream.hpp>
#include <felspar/coro/task.hpp>

#include <set>
#include <vector>


namespace planet::queue {


    /// ## Push producer, multi-consumer queue
    /**
     * A queue which can hold an unlimited number of un-consumed items and
     * provide them to each of the consumers. Once a consumer subscribes to the
     * queue then they are guaranteed to see every value.
     *
     * The value type must be copyable. This type is not thread safe.
     */
    template<typename T>
    class pmc {
        using internal_buffer_type = std::vector<T>;


      public:
        using value_type = T;

        pmc() = default;
        pmc(pmc const &) = delete;
        pmc(pmc &&) = delete;
        pmc &operator=(pmc const &) = delete;
        pmc &operator=(pmc &&) = delete;


        void push(T t) {
            for (auto *s : consumers) {
                s->values.push_back(t);
                if (auto h{std::exchange(s->continuation, {})}; h) {
                    continuations.push_back(h);
                }
            }
            for (auto h : continuations) { h.resume(); }
            continuations.clear();
        }


        class consumer {
            friend class pmc;

            consumer(pmc *s) : self{s} { self->consumers.insert(this); }

            consumer(consumer const &) = delete;
            consumer(consumer &&) = delete;
            consumer &operator=(consumer const &) = delete;
            consumer &operator=(consumer &&) = delete;

            pmc *self;
            felspar::coro::coroutine_handle<> continuation;
            internal_buffer_type values;


          public:
            ~consumer() { self->consumers.erase(this); }

            auto next() {
                struct awaitable {
                    consumer *dr;
                    bool await_ready() const noexcept {
                        return not dr->values.empty();
                    }
                    void await_suspend(
                            felspar::coro::coroutine_handle<> h) noexcept {
                        dr->continuation = h;
                    }
                    T await_resume() {
                        T t = std::move(dr->values.front());
                        dr->values.erase(dr->values.begin());
                        return t;
                    }
                };
                return awaitable{this};
            }
        };
        auto values() { return consumer{this}; }


        felspar::coro::stream<value_type> stream() {
            auto s = values();
            while (true) { co_yield co_await s.next(); }
        }


        template<typename F = value_type>
        felspar::coro::task<void> forward(pmc<F> &q) {
            auto v = values();
            while (true) { q.push(co_await v.next()); }
        }


      private:
        std::set<consumer *> consumers;
        /// Cached memory used by `push`
        std::vector<felspar::coro::coroutine_handle<>> continuations;
    };


}
