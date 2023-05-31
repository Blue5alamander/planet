#pragma once


#include <planet/ecs/entity.hpp>
#include <planet/ecs/entity_id.hpp>

#include <felspar/test/source.hpp>


namespace planet::ecs {


    namespace detail {
        /// ## Abstract base class used for entity look-ups
        struct entity_lookup {
            friend class ecs::entity_id;
            friend class ecs::weak_entity_id;

            [[nodiscard]] virtual entity_id create() = 0;

            [[nodiscard]] virtual detail::entity &
                    entity(std::size_t,
                           felspar::source_location const & =
                                   felspar::source_location::current()) = 0;
            [[nodiscard]] virtual detail::entity &
                    entity(std::size_t,
                           std::size_t,
                           felspar::source_location const & =
                                   felspar::source_location::current()) = 0;
            [[nodiscard]] virtual detail::entity const &entity(
                    std::size_t,
                    std::size_t,
                    felspar::source_location const & =
                            felspar::source_location::current()) const = 0;

            [[nodiscard]] virtual detail::entity *maybe_entity(
                    std::size_t,
                    std::size_t,
                    felspar::source_location const & =
                            felspar::source_location::current()) = 0;

            [[nodiscard]] virtual mask_type &mask_for(
                    std::size_t,
                    std::size_t,
                    std::size_t,
                    felspar::source_location const & =
                            felspar::source_location::current()) = 0;
            [[nodiscard]] virtual mask_type mask_for(
                    std::size_t,
                    std::size_t,
                    std::size_t,
                    felspar::source_location const & =
                            felspar::source_location::current()) const = 0;

          protected:
            virtual void release(std::size_t) {}
            virtual void destroy(std::size_t) = 0;
        };
    }


    /// ## Implementation for `entity_id`
    inline entity_id::entity_id(
            detail::entity_lookup *const o,
            std::size_t const i,
            std::size_t const g,
            felspar::source_location const &loc)
    : owner{o}, generation{g}, id{i} {
        increment(loc);
    }
    inline entity_id::entity_id(entity_id &&o)
    : owner{std::exchange(o.owner, nullptr)},
      generation{std::exchange(o.generation, {})},
      id{std::exchange(o.id, {})} {}
    inline entity_id::entity_id(entity_id const &o)
    : owner{o.owner}, generation{o.generation}, id{o.id} {
        increment();
    }

    inline entity_id::~entity_id() { decrement(); }

    inline void entity_id::increment(felspar::source_location const &loc) {
        if (owner) { owner->entity(id, generation, loc).increment_strong(); }
    }
    inline void entity_id::decrement() {
        if (owner and owner->entity(id, generation).decrement_strong() == 0u) {
            owner->destroy(id);
        }
    }

    inline entity_id &entity_id::operator=(entity_id &&eid) {
        decrement();
        owner = std::exchange(eid.owner, nullptr);
        generation = std::exchange(eid.generation, {});
        id = std::exchange(eid.id, {});
        return *this;
    }
    inline entity_id &entity_id::operator=(entity_id const &eid) {
        decrement();
        owner = eid.owner;
        generation = eid.generation;
        id = eid.id;
        increment();
        return *this;
    }

    inline detail::entity *entity_id::operator->() {
        return &owner->entity(id, generation);
    }
    inline detail::entity const *entity_id::operator->() const {
        return &owner->entity(id, generation);
    }
    inline mask_type &entity_id::mask(std::size_t const storage_index) {
        return owner->mask_for(storage_index, id, generation);
    }
    inline mask_type entity_id::mask(std::size_t const storage_index) const {
        return owner->mask_for(storage_index, id, generation);
    }

    inline entity_id::operator bool() const noexcept {
        if (owner and generation and id) {
            auto &e = owner->entity(id);
            return e.generation == generation;
        } else {
            return false;
        }
    }


}
