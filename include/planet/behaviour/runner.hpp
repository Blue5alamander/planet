#pragma once


#include <planet/behaviour/state.hpp>

#include <felspar/coro/task.hpp>


namespace planet::behaviour {


    class context;


    /// ## Abstract super-class for running a behaviour
    /**
     * End users could generally use one of the construction algorithms:
     *
     * * **Composites**:
     * * **Decorators**: `always_failure`/`always_success`
     * * **Leaves**: `create_leaf`
     *
     * These return a `constexpr` compatible functor that can be run at any time
     * by passing in a suitable context.
     */
    class runner {
      public:
        using result_type = felspar::coro::task<state, context>;
        virtual result_type operator()(context &) const = 0;
    };


}
