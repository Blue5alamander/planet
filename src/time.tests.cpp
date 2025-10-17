#include <planet/time.hpp>
#include <felspar/test.hpp>

#include <felspar/coro/eager.hpp>

#include <array>


using namespace std::literals;


namespace {


    auto const suite = felspar::testsuite("clock");


    auto const basic = suite.test("basic", [](auto check) {
        planet::time::clock clock;
        check(clock.now()) == planet::time::clock::time_point{};
    });


    felspar::coro::task<void> sleeper(planet::time::clock &clock, bool &flag) {
        co_await clock.sleep(10ms);
        flag = true;
    }
    auto const sleep = suite.test("sleep", [](auto check) {
        [](auto check) -> felspar::coro::task<void> {
            bool woke_up = false;
            planet::time::clock clock;

            felspar::coro::eager<> sub_task;
            sub_task.post(sleeper, std::ref(clock), std::ref(woke_up));

            check(woke_up) == false;
            check(clock.advance_by(10ms)) == 1u;
            check(woke_up) == true;

            co_await std::move(sub_task).release();
        }(check)
                                  .get();
    });


    /**
     * The first coroutine that wants to wake up at particular time also needs
     * to be the first one that actually gets resumed.
     */
    felspar::coro::task<void>
            setter(planet::time::clock &clock,
                   std::vector<std::size_t> &output,
                   planet::time::clock::duration const d,
                   std::size_t const v) {
        co_await clock.sleep(d);
        output.push_back(v);
    }
    auto const ordering = suite.test("ordering", [](auto check) {
        std::vector<std::size_t> run_order;
        planet::time::clock clock;

        std::array<felspar::coro::eager<>, 4> sub_tasks;
        sub_tasks[0].post(
                setter, std::ref(clock), std::ref(run_order), 10ms, 0);
        sub_tasks[1].post(
                setter, std::ref(clock), std::ref(run_order), 10ms, 1);
        sub_tasks[2].post(
                setter, std::ref(clock), std::ref(run_order), 10ms, 2);
        sub_tasks[3].post(setter, std::ref(clock), std::ref(run_order), 9ms, 3);
        check(run_order.empty()) == true;

        check(clock.advance_by(10ms)) == 4u;
        check(run_order.size()) == 4u;

        check(run_order[0]) == 3u;
        check(run_order[1]) == 0u;
        check(run_order[2]) == 1u;
        check(run_order[3]) == 2u;
    });


    /**
     * Make sure that sleeping and waking and then sleeping again works as it
     * should.
     */
    auto const sleeping = suite.test("sleeping", [](auto check, auto &log) {
        std::vector<std::size_t> run_order;
        planet::time::clock clock;
        constexpr planet::time::clock::time_point epoch{};

        std::array<felspar::coro::eager<>, 4> sub_tasks;
        sub_tasks[0].post(
                setter, std::ref(clock), std::ref(run_order), 10ms, 0);

        check(clock.advance_one(epoch + 10ms)) == 10ms;
        log << clock.now().time_since_epoch().count() << '\n';
        check(clock.now()) == epoch + 10ms;

        sub_tasks[1].post(
                setter, std::ref(clock), std::ref(run_order), 10ms, 1);
        check(clock.advance_one(epoch + 20ms)) == 10ms;
        check(clock.now()) == epoch + 20ms;

        check(run_order[0]) == 0u;
        check(run_order[1]) == 1u;
    });


}
