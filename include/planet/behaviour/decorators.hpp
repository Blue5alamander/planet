#pragma once


#include <planet/behaviour/runner.hpp>


namespace planet::behaviour {


    /// ## Implementations of various generic behaviours


    /// ### `always_failure`
    constexpr auto always_failure(runner const &run) {
        struct impl : public runner {
            constexpr impl(runner const &r) : run{r} {}
            runner const &run;
            runner::result_type operator()(context &ctx) const override {
                co_await run(ctx);
                co_return state::failure;
            }
        };
        return impl{run};
    };
    /// ### `always_success`
    constexpr auto always_success(runner const &run) {
        struct impl : public runner {
            constexpr impl(runner const &r) : run{r} {}
            runner const &run;
            runner::result_type operator()(context &ctx) const override {
                co_await run(ctx);
                co_return state::success;
            }
        };
        return impl{run};
    };


}
