#pragma once


#include <planet/ecs/forward.hpp>
#include <planet/serialise.hpp>

#include <felspar/test/source.hpp>

#include <string_view>
#include <utility>
#include <variant>


namespace planet::ecs {


    /// ## Entity Identifier
    /**
     * An entity reference. Keeps the entity alive.
     *
     * The implementation can be found in
     * [entity_lookup.hpp](./entity_lookup.hpp).
     */
    class entity_id final {
        template<typename... Storages>
        friend class entities;
        friend class weak_entity_id;

        detail::entity_lookup *owner = nullptr;

        void increment(
                felspar::source_location const & =
                        felspar::source_location::current());
        void decrement();

        std::size_t generation = {};
        std::size_t m_id = {};


      public:
        entity_id() = default;
        explicit entity_id(
                detail::entity_lookup *,
                std::size_t,
                std::size_t,
                felspar::source_location const & =
                        felspar::source_location::current());
        entity_id(entity_id &&);
        entity_id(entity_id const &);
        ~entity_id();

        entity_id &operator=(entity_id &&);
        entity_id &operator=(entity_id const &);

        std::size_t id() const noexcept { return m_id; };

        std::variant<std::size_t, std::string_view> name_or_id() const;

        detail::entity *operator->();
        detail::entity const *operator->() const;

        mask_type &
                mask(std::size_t,
                     felspar::source_location const & =
                             felspar::source_location::current());
        mask_type
                mask(std::size_t,
                     felspar::source_location const & =
                             felspar::source_location::current()) const;

        friend bool operator==(entity_id const &, entity_id const &) noexcept =
                default;

        explicit operator bool() const noexcept;


        void log(serialise::save_buffer &sb) const {
            std::visit([&sb](auto const &v) { save(sb, v); }, name_or_id());
        }
    };


}
