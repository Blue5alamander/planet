#pragma once


#include <planet/ecs/entity.hpp>
#include <planet/ecs/entity_id.hpp>


namespace planet::ecs {


    namespace detail {
        /// ## Abstract base class used for entity look-ups
        struct entity_lookup {
            friend class ecs::entity_id;

            [[nodiscard]] virtual entity_id create() = 0;

            [[nodiscard]] virtual detail::entity &entity(std::size_t) = 0;
            [[nodiscard]] virtual detail::entity const &
                    entity(std::size_t) const = 0;

          protected:
            virtual void destroy(std::size_t) = 0;
        };
    }


    /// ## Implementation for `entity_id`
    inline entity_id::entity_id(
            detail::entity_lookup *const o, std::size_t const i)
    : owner{o}, id{i} {
        increment();
    }
    inline entity_id::entity_id(entity_id &&o)
    : owner{std::exchange(o.owner, nullptr)}, id{std::exchange(o.id, {})} {}
    inline entity_id::entity_id(entity_id const &o) : owner{o.owner}, id{o.id} {
        increment();
    }

    inline entity_id::~entity_id() { decrement(); }

    inline void entity_id::increment() {
        if (owner) { owner->entity(id).increment_strong(); }
    }
    inline void entity_id::decrement() {
        if (owner and owner->entity(id).decrement_strong() == 0u) {
            owner->destroy(id);
        }
    }

    inline entity_id &entity_id::operator=(entity_id &&eid) {
        decrement();
        owner = std::exchange(eid.owner, nullptr);
        id = std::exchange(eid.id, {});
        return *this;
    }
    inline entity_id &entity_id::operator=(entity_id const &eid) {
        decrement();
        owner = eid.owner;
        id = eid.id;
        increment();
        return *this;
    }

    inline detail::entity *entity_id::operator->() {
        return &owner->entity(id);
    }
    inline detail::entity const *entity_id::operator->() const {
        return &owner->entity(id);
    }


}
