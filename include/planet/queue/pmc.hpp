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
        pmc(pmc &&p) : consumers{std::move(p.consumers)} {
            for (auto &c : consumers) { c->self = this; }
        }
        pmc &operator=(pmc const &) = delete;
        pmc &operator=(pmc &&p) {
            consumers = std::move(p.consumers);
            for (auto &c : consumers) { c->self = this; }
            return *this;
        }
        ~pmc() {
            for (auto &c : consumers) { c->self = nullptr; }
        }


        void push(T t) {
            for (std::size_t idx{}; idx < consumers.size();) {
                auto *consumer = consumers[idx];
                consumer->values.push_back(t);
                if (auto h{std::exchange(consumer->continuation, {})}; h) {
                    h.resume();
                }
                if (idx < consumers.size() and consumers[idx] == consumer) {
                    ++idx;
                }
            }
        }


        class consumer {
            friend class pmc;

            consumer(pmc *s) : self{s} { self->consumers.push_back(this); }

            consumer(consumer const &) = delete;
            consumer &operator=(consumer const &) = delete;
            consumer &operator=(consumer &&) = delete;

            pmc *self;
            felspar::coro::coroutine_handle<> continuation;
            internal_buffer_type values;


          public:
            consumer(consumer &&other)
            : self{std::exchange(other.self, nullptr)},
              continuation{std::exchange(other.continuation, {})},
              values{std::move(other.values)} {
                if (self) {
                    for (auto &c : self->consumers) {
                        if (c == &other) { c = this; }
                    }
                }
            }
            ~consumer() {
                if (self) { std::erase(self->consumers, this); }
            }

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
        std::vector<consumer *> consumers;
    };


}
