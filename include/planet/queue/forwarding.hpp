#pragma once


#include <planet/queue/forward.hpp>

#include <felspar/coro/task.hpp>


namespace planet::queue {


    template<typename Queue, typename Stream>
    inline felspar::coro::task<void> forward_values(Queue &q, Stream s) {
        while (true) { q.push(co_await s.next()); }
    }


}
