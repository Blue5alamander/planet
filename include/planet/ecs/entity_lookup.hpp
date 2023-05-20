#pragma once


#include <planet/ecs/entity.hpp>
#include <planet/ecs/entity_id.hpp>


namespace planet::ecs {


    /// ## Abstract base class used for entity look-ups
    struct entity_lookup {
        virtual entity_id create() = 0;
        virtual ecs::entity &entity(std::size_t) = 0;
    };


    inline entity_id::entity_id(entity_lookup *const o, std::size_t const i)
    : owner{o}, id{i} {
        ++owner->entity(id).reference_count;
    }
    inline entity_id::entity_id(entity_id const &o) : owner{o.owner}, id{o.id} {
        if (owner) { ++owner->entity(id).reference_count; }
    }
    inline entity_id::~entity_id() {
        if (owner) { --owner->entity(id).reference_count; }
    }
    inline entity *entity_id::operator->() { return &owner->entity(id); }


}
