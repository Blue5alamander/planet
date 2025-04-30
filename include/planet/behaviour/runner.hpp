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
        using context_type = context;
        using state_type = state<R>;
        using result_type = felspar::coro::task<state_type, context_type>;
        virtual result_type operator()(context &) const = 0;
    };


}
