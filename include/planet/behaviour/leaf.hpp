#pragma once


#include <planet/behaviour/runner.hpp>


namespace planet::behaviour {


    /// ## Leaf behaviours

    /// ### Create a leaf behaviour from a nullary coroutine
    template<typename R>
    constexpr auto leaf(typename runner<R>::result_type (*c)()) {
        struct impl : public runner<R> {
            constexpr impl(runner<R>::result_type (*c)()) : run{c} {}
            runner<R>::result_type (*run)();
            runner<R>::result_type operator()(context &) const override {
                co_return co_await run();
            }
        };
        return impl{c};
    }
    template<typename R, typename... Param, typename... Arg>
    constexpr auto
            leaf(typename runner<R>::result_type (*c)(Param...),
                 parameter<Arg> const &...arg) {
        using run_fn = runner<R>::result_type (*)(Param...);

        static_assert(
                sizeof...(Param) == sizeof...(Arg),
                "Pass one `parameter` instance per argument of the coroutine");
        using param_types = std::tuple<Param...>;
        using arg_types = std::tuple<parameter<Arg>...>;

        struct impl : public runner<R> {
            constexpr impl(run_fn c, parameter<Arg> const &...a)
            : run{c}, args{a...} {}
            run_fn run;
            std::tuple<parameter<Arg> const &...> args;

            FELSPAR_CORO_WRAPPER runner<R>::result_type
                    operator()(context &ctx) const override {
                []<std::size_t... Is>(std::index_sequence<Is...>) {
                    static_assert(
                            (...
                             and compatible_parameters<
                                     std::tuple_element_t<Is, param_types>,
                                     std::tuple_element_t<Is, arg_types>>));
                }(std::index_sequence_for<Param...>{});

                std::tuple<context &, run_fn> cr{ctx, run};
                return std::apply(impl::dispatch, std::tuple_cat(cr, args));
            }
            static runner<R>::result_type dispatch(
                    context &ctx, run_fn run, parameter<Arg> const &...a) {
                co_return co_await run(ctx.look_up(a)...);
            }
        };
        return impl{c, arg...};
    }


}
