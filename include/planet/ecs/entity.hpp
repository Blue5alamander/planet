#pragma once


#include <planet/ecs/forward.hpp>

#include <felspar/coro/coroutine.hpp>

#include <vector>


namespace planet::ecs::detail {


    /// ## Entity
    /// Each entity is comprised of a set of components
    class entity final {
        template<typename... Storages>
        friend class ecs::entities;
        friend class ecs::entity_id;
        template<typename... Components>
        friend class ecs::storage;
        friend class ecs::weak_entity_id;

        /// ### The generation for this entity
        std::size_t generation = 0;

        /// ### Strong reference count
        std::size_t strong_count = 0;
        void increment_strong() noexcept { ++strong_count; }
        auto decrement_strong() noexcept { return --strong_count; }


      public:
        explicit entity() {}


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
