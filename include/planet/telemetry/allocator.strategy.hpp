#pragma once


#include <planet/telemetry/map.hpp>

#include <felspar/memory/concepts.hpp>

#include <utility>


namespace planet::telemetry {


    /// ## Allocator telemetry wrapper
    /**
     * Wraps any allocator strategy to record allocation size histograms into
     * the planet telemetry system. Each allocation size is counted in a map
     * which can be saved and loaded with other game telemetry.
     *
     * The wrapper itself satisfies `allocator_strategy` and can be used with
     * `pmr_resource` or `felspar::memory::allocator`.
     */
    template<felspar::memory::allocator_strategy Strategy>
    struct allocator_strategy final {
        /// ### Construction

        /// #### Construct from a strategy (for movable strategies)
        allocator_strategy(
                std::string_view const name,
                Strategy s,
                std::source_location const loc = std::source_location::current())
        : strategy{std::move(s)}, telemetry{name, loc} {}

        /// #### Variadic forwarding constructor for in-place construction
        template<typename... Args>
        allocator_strategy(
                std::string_view const name,
                Args &&...args,
                std::source_location const loc = std::source_location::current())
        : strategy{std::forward<Args>(args)...}, telemetry{name, loc} {}

        /// #### Non-copyable
        allocator_strategy(allocator_strategy const &) = delete;
        allocator_strategy &operator=(allocator_strategy const &) = delete;

        /// #### Movable only if the strategy is movable
        allocator_strategy(allocator_strategy &&) noexcept(
                std::is_nothrow_move_constructible_v<Strategy>)
            requires(std::is_move_constructible_v<Strategy>)
        = default;
        allocator_strategy &operator=(allocator_strategy &&) noexcept(
                std::is_nothrow_move_assignable_v<Strategy>)
            requires(std::is_move_assignable_v<Strategy>)
        = default;


        Strategy strategy;
        map<std::size_t, std::size_t> telemetry;


        /// ### Allocation
        [[nodiscard]] std::byte *allocate(std::size_t const bytes) {
            telemetry.update(bytes, 1u, [](auto &n) { ++n; });
            return strategy.allocate(bytes);
        }


        /// ### Deallocation
        void deallocate(void *const ptr, std::size_t const bytes) {
            strategy.deallocate(ptr, bytes);
        }
    };


}
