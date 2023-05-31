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
            if (owner) { owner->entity(m_id, generation).increment_weak(); }
        }
        void decrement() {
            if (owner
                and owner->entity(m_id).decrement_weak(generation) == 0u) {
                owner->release(m_id);
            }
        }

        std::size_t generation = {};
        std::size_t m_id = {};

      public:
        weak_entity_id() = default;
        weak_entity_id(entity_id const &eid)
        : owner{eid.owner}, generation{eid.generation}, m_id{eid.m_id} {
            increment();
        }
        weak_entity_id(weak_entity_id &&w)
        : owner{std::exchange(w.owner, nullptr)},
          generation{std::exchange(w.generation, {})},
          m_id{std::exchange(w.m_id, {})} {}
        weak_entity_id(weak_entity_id const &w)
        : owner{w.owner}, generation{w.generation}, m_id{w.m_id} {
            increment();
        }
        ~weak_entity_id() { decrement(); }

        std::size_t id() const noexcept { return m_id; }

        weak_entity_id &operator=(weak_entity_id &&w) {
            decrement();
            owner = std::exchange(w.owner, nullptr);
            generation = std::exchange(w.generation, {});
            m_id = std::exchange(w.m_id, {});
            return *this;
        }
        weak_entity_id &operator=(weak_entity_id const &);

        auto lock() const {
            if (owner and m_id and generation
                and owner->maybe_entity(m_id, generation)) {
                return entity_id{owner, m_id, generation};
            } else {
                return entity_id{};
            }
        }

        friend bool operator==(
                weak_entity_id const &,
                weak_entity_id const &) noexcept = default;
    };


}
