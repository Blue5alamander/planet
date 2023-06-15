#include <planet/clock.hpp>

#include <planet/serialise/chrono.hpp>


/// ## `planet`


void planet::save(serialise::save_buffer &ab, clock const &c) {
    ab.save_box("_p:clock", c.time);
}
void planet::load(serialise::box &box, clock &c) {
    box.named("_p:clock", c.time);
}


/// ## `planet::clock`


planet::clock::clock() {}


auto planet::clock::sleep(duration const d) -> awaitable {
    return {*this, now() + d};
}


auto planet::clock::advance_one(time_point const latest) -> duration {
    if (not time_line.empty() and time_line.front().when <= latest) {
        auto const d = time_line.front().when - time;
        advance_to(time_line.front().when);
        return d;
    } else {
        return {};
    }
}


std::size_t planet::clock::advance_to(time_point const latest) {
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


/// ## `planet::clock::awaitable`


void planet::clock::awaitable::await_suspend(
        felspar::coro::coroutine_handle<> const h) {
    auto const pos = std::upper_bound(
            clock.time_line.begin(), clock.time_line.end(), wake_up_time);
    clock.time_line.insert(pos, {wake_up_time, h});
}
