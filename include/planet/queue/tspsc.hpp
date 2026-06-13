#pragma once


#include <span>
#include <mutex>
#include <vector>


namespace planet::queue {


    /// ## Multi-producer, single consumer queue
    /**
     * The queue can have data pushed into it from multiple threads, but all
     * subscribers must be on a single thread. Pushing data into the queue is
     * non-blocking, but does acquire a mutex. The consumer thread only holds
     * the mutex for long enough to do a pointer swap on the vector which acts
     * as a queue.
     */
    template<typename T>
    class tspsc {
        std::mutex mtx;
        std::vector<T> queue, consuming;

      public:
        using value_type = T;


        /// ### Reserve capacity
        void reserve(std::size_t const n)
        /**
         * Pre-allocate the internal buffers so that `push` does not allocate in
         * steady state, which would otherwise let a producer block the consumer
         * on a heap allocation while holding the mutex. Call before the
         * consumer starts running.
         */
        {
            std::scoped_lock _{mtx};
            queue.reserve(n);
            consuming.reserve(n);
        }


        /// ### Push data
        void push(value_type t)
        /**
         * Push data to the end of the queue. This process will block to acquire
         * a mutex on the internal queue.
         */
        {
            std::scoped_lock _{mtx};
            queue.push_back(std::move(t));
        }


        /// ### Process data
        std::span<value_type> consume()
        /**
         * Return data held by the queue. The process will block to acquire a
         * mutex on the internal queue. The returned `span`'s data is valid
         * until the next call to `consume`.
         */
        {
            consuming.clear();
            {
                std::scoped_lock _{mtx};
                std::swap(queue, consuming);
            }
            return consuming;
        }
    };


}
