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
    class mpsc {
        std::mutex mtx;
        std::vector<T> queue, consuming;

      public:
        using value_type = T;

        /// ## Push data
        /**
         * Push data to the end of the queue. This process will block to acquire
         * a mutex on the internal queue.
         */
        void push(value_type t) {
            std::scoped_lock _{mtx};
            queue.push_back(std::move(t));
        }

        /// ## Process data
        /**
         * Return data held by the queue. The process will block to acquire a
         * mutex on the internal queue. The returned `span`'s data is valid
         * until the next call to `consume`.
         */
        std::span<value_type> consume() {
            {
                std::scoped_lock _{mtx};
                std::swap(queue, consuming);
            }
            return consuming;
        }
    };


}
