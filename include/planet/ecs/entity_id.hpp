#pragma once


#include <planet/ecs/forward.hpp>

#include <utility>


namespace planet::ecs {


    /// ## Entity Identifier
    /**
     * An entity reference. Keeps the entity alive.
     *
     * The implementation can be found in
     * [entity_lookup.hpp](./entity_lookup.hpp).
     */
    class entity_id final {
        friend class weak_entity_id;

        detail::entity_lookup *owner = nullptr;

        void increment();
        void decrement();

        std::size_t generation = {};

      public:
        entity_id() = default;
        explicit entity_id(detail::entity_lookup *, std::size_t, std::size_t);
        entity_id(entity_id &&);
        entity_id(entity_id const &);
        ~entity_id();

        entity_id &operator=(entity_id &&);
        entity_id &operator=(entity_id const &);

        /// TODO protect this value
        std::size_t id = {};

        detail::entity *operator->();
        detail::entity const *operator->() const;

        friend bool operator==(entity_id const &, entity_id const &) noexcept =
                default;

        explicit operator bool() const noexcept;
    };


}
