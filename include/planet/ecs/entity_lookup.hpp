#pragma once


#include <planet/ecs/entity.hpp>
#include <planet/ecs/entity_id.hpp>
#include <planet/telemetry/id.hpp>

#include <felspar/test/source.hpp>


namespace planet::ecs {


    namespace detail {
        /// ## Abstract base class used for entity look-ups
        struct entity_lookup {
            friend class ecs::entity_id;
            friend class ecs::weak_entity_id;

            [[nodiscard]] virtual entity_id create() = 0;
            [[nodiscard]] virtual entity_id create(std::string) = 0;

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

            [[nodiscard]] virtual std::optional<telemetry::id> const &
                    id(std::size_t) const = 0;

            virtual void
                    kill(entity_id const &,
                         felspar::source_location const & =
                                 felspar::source_location::current()) = 0;


          protected:
            /// ### Lifetime

            /// #### Acquire/release strong/weak counts
            virtual void
                    acquire(entity_id const &,
                            felspar::source_location const & =
                                    felspar::source_location::current()) = 0;
            virtual void release(entity_id const &) noexcept = 0;

            /// #### Force destroy the entity
            virtual void destroy(entity_id const &) = 0;
        };
    }


    /// ## Implementation for `entity_id`
    inline entity_id::entity_id(
            detail::entity_lookup *const o,
            std::size_t const i,
            std::size_t const g,
            felspar::source_location const &loc)
    : owner{o}, generation{g}, m_id{i} {
        increment(loc);
    }
    inline entity_id::entity_id(entity_id &&o) noexcept
    : owner{std::exchange(o.owner, nullptr)},
      generation{std::exchange(o.generation, {})},
      m_id{std::exchange(o.m_id, {})} {}
    inline entity_id::entity_id(entity_id const &o)
    : owner{o.owner}, generation{o.generation}, m_id{o.m_id} {
        increment();
    }

    inline entity_id::~entity_id() { decrement(); }

    inline void entity_id::increment(felspar::source_location const &loc) {
        if (owner) { owner->acquire(*this, loc); }
    }
    inline void entity_id::decrement() noexcept {
        if (owner) { owner->release(*this); }
    }

    inline entity_id &entity_id::operator=(entity_id &&eid) noexcept {
        decrement();
        owner = std::exchange(eid.owner, nullptr);
        generation = std::exchange(eid.generation, {});
        m_id = std::exchange(eid.m_id, {});
        return *this;
    }
    inline entity_id &entity_id::operator=(entity_id const &eid) {
        decrement();
        owner = eid.owner;
        generation = eid.generation;
        m_id = eid.m_id;
        increment();
        return *this;
    }

    inline std::variant<std::size_t, std::string_view>
            entity_id::name_or_id() const {
        if (not owner) {
            return m_id;
        } else if (auto const &name = owner->id(m_id); name) {
            return std::string_view{name->name()};
        } else {
            return m_id;
        }
    }
    inline detail::entity *entity_id::operator->() {
        return &owner->entity(m_id, generation);
    }
    inline detail::entity const *entity_id::operator->() const {
        return &owner->entity(m_id, generation);
    }
    inline mask_type &entity_id::mask(
            std::size_t const storage_index,
            felspar::source_location const &loc) {
        return owner->mask_for(storage_index, m_id, generation, loc);
    }
    inline mask_type entity_id::mask(
            std::size_t const storage_index,
            felspar::source_location const &loc) const {
        return owner->mask_for(storage_index, m_id, generation, loc);
    }

    inline entity_id::operator bool() const noexcept {
        if (owner and generation and m_id) {
            auto &e = owner->entity(m_id);
            return e.generation == generation;
        } else {
            return false;
        }
    }


}
