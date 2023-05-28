#pragma once


#include <planet/ecs/entity_lookup.hpp>


namespace planet::ecs {


    /// ## A weak Identifier reference
    /**
     * A weak entity reference. Does not keep the entity alive.
     */
    class weak_entity_id final {
        detail::entity_lookup *owner = nullptr;

        void increment() {
            if (owner) { owner->entity(id, generation).increment_weak(); }
        }
        void decrement() {
            if (owner and owner->entity(id).decrement_weak(generation) == 0u) {
                owner->release(id);
            }
        }

        std::size_t generation = {};
        std::size_t id = {};

      public:
        weak_entity_id() = default;
        weak_entity_id(entity_id const &eid)
        : owner{eid.owner}, generation{eid.generation}, id{eid.id} {
            increment();
        }
        weak_entity_id(weak_entity_id &&w)
        : owner{std::exchange(w.owner, nullptr)},
          generation{std::exchange(w.generation, {})},
          id{std::exchange(w.id, {})} {}
        weak_entity_id(weak_entity_id const &w)
        : owner{w.owner}, generation{w.generation}, id{w.id} {
            increment();
        }
        ~weak_entity_id() { decrement(); }

        weak_entity_id &operator=(weak_entity_id &&w) {
            decrement();
            owner = w.owner;
            generation = w.generation;
            id = w.id;
            increment();
            return *this;
        }
        weak_entity_id &operator=(weak_entity_id const &);

        auto lock() {
            if (owner->maybe_entity(id, generation)) {
                return entity_id{owner, id, generation};
            } else {
                return entity_id{};
            }
        }

        friend bool operator==(
                weak_entity_id const &,
                weak_entity_id const &) noexcept = default;
    };


}
