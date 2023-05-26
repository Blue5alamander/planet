#pragma once


#include <planet/ecs/forward.hpp>

#include <felspar/coro/coroutine.hpp>

#include <vector>


namespace planet::ecs::detail {


    /// ## Entity
    /// Each entity is comprised of a set of components
    class entity final {
        friend class ecs::entity_id;
        template<typename... Components>
        friend class ecs::storage;

        std::size_t strong_count = 0;
        void increment_strong() { ++strong_count; }
        auto decrement_strong() { return --strong_count; }

      public:
        explicit entity(std::size_t const component_count)
        : components(component_count) {}

        /// Bitset of the components that this entity uses in each of the
        /// storages
        /// TODO Should be a std::array, or should it? Span is probably best and
        /// leave storage in the entities where this instance is also stored
        std::vector<mask_type> components = {};

        /**
        TODO Add in an awaitable system for the below
        ```cpp
        struct on_destroy_continuation {
            mask_type mask;
            felspar::coro::coroutine_handle<> continuation;
        };
        std::vector<on_destroy_continuation> destroy_continuations;
        auto co_on_destroy(mask_type const mask) {
            struct awaitable {
                ecs::entity &entity;
                mask_type mask;

                bool await_ready() const noexcept { return false; }
                void await_suspend(felspar::coro::coroutine_handle<> h) noexcept
        { entity.destroy_continuations.push_back({mask, h});
                }
                void await_resume() const noexcept {}
            };
            return awaitable{*this, mask};
        }
        ```
        */
    };


}
