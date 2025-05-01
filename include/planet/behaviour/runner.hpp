#pragma once


#include <planet/behaviour/state.hpp>

#include <felspar/coro/task.hpp>


namespace planet::behaviour {


    class context;


    /// ## Abstract super-class for running a behaviour
    /**
     * Behaviours either return a value (which may be `void`), or they return a
     * failure.
     *
     * End users should generally use one of the construction algorithms:
     *
     * * **Composites**:
     * * **Decorators**: `always_failure`/`always_success`
     * * **Leaves**: `create_leaf`
     *
     * These return a `constexpr` compatible functor that can be run at any time
     * by passing in a suitable context.
     */
    template<typename R>
    class runner {
      public:
        using state_type = state<R>;
        struct result_type {
            using value_type = R;


            result_type(failure f) : state{std::unexpect_t{}, std::move(f)} {}
            result_type(value_type r)
            : state{std::in_place_t{}, std::move(r)} {}


            state_type state;
        };
        using co_result_type = felspar::coro::task<result_type, context_base>;


        virtual co_result_type operator()(context_base &) const = 0;
    };


}
