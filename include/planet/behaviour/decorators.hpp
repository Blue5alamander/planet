#pragma once


#include <planet/behaviour/runner.hpp>


namespace planet::behaviour {


    /// ## Implementations of various generic behaviours


    /// ### `always_failure`
    /**
     * Ignores the actual result of the runner and returns failure no matter
     * what the runner does.
     */
    template<typename R>
    constexpr auto always_failure(runner<R> const &run, failure reason = {}) {
        struct impl : public runner<R> {
            constexpr impl(runner<R> const &rr, failure rs)
            : run{rr}, reason{std::move(rs)} {}
            runner<R> const &run;
            failure reason;
            runner<R>::result_type operator()(context &ctx) const override {
                co_await run(ctx);
                co_return reason;
            }
        };
        return impl{run, std::move(reason)};
    };
    /// ### `always_success`
    /**
     * Ignores the actual result of the runner and returns a fixed value no
     * matter what the runner does.
     */
    template<typename R, typename V>
    constexpr auto always_success(runner<R> const &run, V &&result) {
        struct impl : public runner<R> {
            constexpr impl(runner<R> const &r, V v)
            : run{r}, result{std::forward<V>(v)} {}
            runner<R> const &run;
            runner<R>::result_type result;
            runner<R>::result_type operator()(context &ctx) const override {
                co_await run(ctx);
                co_return result;
            }
        };
        return impl{run, std::forward<V>(result)};
    };


}
