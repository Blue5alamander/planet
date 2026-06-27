#pragma once


#include <felspar/io/warden.hpp>
#include <felspar/memory/small_vector.hpp>

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
     *
     * This type cannot be used to create a coroutine, but rather is used to
     * facilitate communication to coroutines.
     */
    template<typename T>
    class pmc {
        using internal_buffer_type = std::vector<T>;
        std::size_t value_push_count = {};


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


        std::size_t consumer_count() const { return consumers.size(); }


        /// ### Pushing values

        /// #### Synchronous push
        void push(T t)
        /**
         * A synchronous push of data into a consumer may lead to the death of
         * the consumer. By iterating by index we can sidestep that problem, but
         * it also requires a little dance in how the index is incremented.
         *
         * If the resumption of the coroutines held could lead to the
         * destruction of the object calling `push` then using this synchronous
         * push will lead to undefined behaviour. In those circumstances use the
         * asynchronous overload.
         */
        {
            ++value_push_count;
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

        /// #### Asynchronous push
        void push(felspar::io::warden &ward, T t)
        /**
         * The consumers are woken up asynchronously. This should be used in
         * cases where the resumption of a consumer might lead to the
         * destruction of the object that is performing a push (e.g. a button
         * which dismisses when the button is pressed).
         *
         * We don't need to worry about the death of consumers in this case as
         * the resume happens asynchronously, so after the code that calls this
         * `push` overload has completed and it blocks again on another
         * awaitable.
         */
        {
            ++value_push_count;
            /**
             * Wake the consumers in a single batched `async_resume` rather than
             * one call each. The `small_vector` keeps this allocation free; if
             * there are ever more consumers than it can hold we flush what we
             * have so far and carry on, so any number of consumers is handled.
             */
            felspar::memory::small_vector<std::coroutine_handle<>> continuations;
            for (auto *consumer : consumers) {
                consumer->values.push_back(t);
                if (auto h{std::exchange(consumer->continuation, {})}; h) {
                    if (not continuations.has_room()) {
                        ward.async_resume(continuations);
                        continuations.clear();
                    }
                    continuations.push_back(h);
                }
            }
            if (not continuations.empty()) { ward.async_resume(continuations); }
        }


        class consumer {
            friend class pmc;

            consumer(pmc *s) : self{s} { self->consumers.push_back(this); }

            consumer(consumer const &) = delete;
            consumer &operator=(consumer const &) = delete;
            consumer &operator=(consumer &&) = delete;

            pmc *self;
            std::coroutine_handle<> continuation;
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
                    void await_suspend(std::coroutine_handle<> h) noexcept {
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
        auto values_pushed() const noexcept { return value_push_count; }


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
