#pragma once


#include <planet/telemetry/allocator.strategy.hpp>

#include <felspar/memory/new_delete.strategy.hpp>
#include <felspar/memory/pool.strategy.hpp>


namespace planet::telemetry {


    /// ## Pool strategy with a telemetry-tracked fallback
    /**
     * Mirrors `felspar::memory::pool<Sizes...>` but fixes the fallback to a
     * `planet::telemetry::allocator_strategy<felspar::memory::new_delete_strategy>`
     * so that allocations which escape the pool tiers are counted separately
     * from those served by the pool.
     *
     * Construct via the factory-lambda overload of `pool_strategy`, passing a
     * callable that returns a named fallback instance:
     *
     * ```cpp
     * planet::telemetry::pool<8, 64, 512> ps{[] {
     *     return planet::telemetry::allocator_strategy<
     *             felspar::memory::new_delete_strategy>{"my_pool_fallback"};
     * }};
     * ```
     */
    template<std::size_t... Sizes>
    using pool = felspar::memory::pool_strategy<
            std::array<std::size_t, sizeof...(Sizes)>{Sizes...},
            allocator_strategy<felspar::memory::new_delete_strategy>>;


}
