#pragma once


#include <planet/ecs/forward.hpp>

#include <utility>


namespace planet::ecs {


    /// ## Entity Identifier
    /// An entity reference. Keeps the entity alive
    class entity_id final {
        template<typename... Components>
        friend class entity_storage;
        entity_lookup *owner = nullptr;

      public:
        entity_id() = default;
        entity_id(entity_lookup *, std::size_t);
        entity_id(entity_id &&o) : owner{std::exchange(o.owner, nullptr)} {}
        entity_id(entity_id const &);
        ~entity_id();

        entity_id &operator=(entity_id &&);
        entity_id &operator=(entity_id const &);

        /// TODO protect this value
        std::size_t id;

        entity *operator->();
    };


}
