#pragma once


#include <planet/ecs/entity.hpp>
#include <planet/ecs/entity_id.hpp>


namespace planet::ecs {


    namespace detail {
        /// ## Abstract base class used for entity look-ups
        struct entity_lookup {
            [[nodiscard]] virtual entity_id create() = 0;
            [[nodiscard]] virtual detail::entity &entity(std::size_t) = 0;
            [[nodiscard]] virtual detail::entity const &
                    entity(std::size_t) const = 0;
        };
    }


    /// ## Implementation for `entity_id`
    inline entity_id::entity_id(
            detail::entity_lookup *const o, std::size_t const i)
    : owner{o}, id{i} {
        ++owner->entity(id).reference_count;
    }
    inline entity_id::entity_id(entity_id &&o)
    : owner{std::exchange(o.owner, nullptr)}, id{std::exchange(o.id, {})} {}
    inline entity_id::entity_id(entity_id const &o) : owner{o.owner}, id{o.id} {
        if (owner) { ++owner->entity(id).reference_count; }
    }

    inline entity_id::~entity_id() {
        if (owner) { --owner->entity(id).reference_count; }
    }

    inline entity_id &entity_id::operator=(entity_id &&eid) {
        if (owner) { --owner->entity(id).reference_count; }
        owner = std::exchange(eid.owner, nullptr);
        id = std::exchange(eid.id, {});
        return *this;
    }
    inline entity_id &entity_id::operator=(entity_id const &eid) {
        if (owner) { --owner->entity(id).reference_count; }
        owner = eid.owner;
        id = eid.id;
        if (owner) { ++owner->entity(id).reference_count; }
        return *this;
    }

    inline detail::entity *entity_id::operator->() {
        return &owner->entity(id);
    }
    inline detail::entity const *entity_id::operator->() const {
        return &owner->entity(id);
    }


}
