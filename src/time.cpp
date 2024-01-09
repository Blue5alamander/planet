#include <planet/time/clock.hpp>
#include <planet/time/rate-limiter.hpp>

#include <planet/serialise/chrono.hpp>

#include <algorithm>


/// ## `planet::time`


void planet::time::save(serialise::save_buffer &ab, clock const &c) {
    ab.save_box("_p:clock", c.time);
}
void planet::time::load(serialise::box &box, clock &c) {
    box.named("_p:clock", c.time);
}


/// ## `planet::time::clock`


planet::time::clock::clock() {}


auto planet::time::clock::sleep(duration const d) -> awaitable {
    return {*this, now() + d};
}


auto planet::time::clock::wake_at(time_point const tp) -> awaitable {
    return {*this, tp};
}


auto planet::time::clock::advance_one(time_point const latest) -> duration {
    if (not time_line.empty() and time_line.front().when <= latest) {
        auto const d = time_line.front().when - time;
        advance_to(time_line.front().when);
        return d;
    } else {
        return {};
    }
}


std::size_t planet::time::clock::advance_to(time_point const latest) {
    std::size_t run{};
    while (not time_line.empty() and time_line.front().when <= latest) {
        time = time_line.front().when;
        time_line.front().continuation.resume();
        time_line.erase(time_line.begin());
        ++run;
    }
    time = latest;
    return run;
}


/// ## `planet::time::clock::awaitable`


planet::time::clock::awaitable::awaitable(awaitable &&a)
: clock{a.clock},
  wake_up_time{a.wake_up_time},
  continuation{std::exchange(a.continuation, {})} {}


planet::time::clock::awaitable::~awaitable() {
    if (continuation) {
        std::erase(
                clock.time_line,
                time::clock::event{wake_up_time, continuation});
    }
}


void planet::time::clock::awaitable::await_suspend(
        felspar::coro::coroutine_handle<> const h) {
    continuation = h;
    auto const pos = std::upper_bound(
            clock.time_line.begin(), clock.time_line.end(), wake_up_time);
    clock.time_line.insert(pos, {wake_up_time, h});
}


// ## `planet::time::time_limiter`


planet::time::time_limiter::time_limiter(std::chrono::nanoseconds const ns)
: frame_time{ns} {}


std::chrono::nanoseconds planet::time::time_limiter::wait_time() {
    ++frame_number;
    auto const wait_until = base_time + frame_time * frame_number;
    return wait_until - std::chrono::steady_clock::now();
}
