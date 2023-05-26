#pragma once


#include <planet/ecs/component_proxy.hpp>
#include <planet/ecs/entities.hpp>
#include <planet/ecs/type_index.hpp>

#include <felspar/memory/holding_pen.hpp>


namespace planet::ecs {


    namespace detail {
        [[noreturn]] void throw_no_entities_instance(
                felspar::source_location const & =
                        felspar::source_location::current());
        [[noreturn]] void throw_component_type_not_valid(
                felspar::source_location const & =
                        felspar::source_location::current());
        [[noreturn]] void throw_component_not_present(
                felspar::source_location const & =
                        felspar::source_location::current());
    }


    /// ## Holder for all entities
    template<typename... Components>
    class storage final {
        template<typename Component, typename... Cs>
        friend class component_proxy;
        template<typename... Storages>
        friend class entities;

        void assert_entities() const {
            if (not entities or not entities_storage_index) {
                detail::throw_no_entities_instance();
            }
        }

      public:
        storage() = default;

        using component_tuple = std::tuple<Components...>;
        static constexpr std::size_t component_count = sizeof...(Components);
        using indexes = std::index_sequence_for<Components...>;

        template<typename L>
        static constexpr std::optional<std::size_t> maybe_component_index() {
            return detail::component_index_sequence<L, Components...>(
                    std::make_index_sequence<sizeof...(Components)>{});
        }
        template<typename L>
        static constexpr std::size_t component_index() {
            constexpr auto ci =
                    detail::component_index_sequence<L, Components...>(
                            std::make_index_sequence<sizeof...(Components)>{});
            static_assert(ci.has_value());
            return *ci;
        }

        /// ### Add a new slot to the back of each store
        void emplace_back() {
            [this]<std::size_t... Is>(std::index_sequence<Is...>) {
                (std::get<Is>(components).emplace_back(), ...);
            }
            (indexes{});
        }


        /// ### Create an entity with the desired components
        template<typename... Cs>
        entity_id create(Cs &&...cs) {
            assert_entities();
            auto eid = entities->create();
            (add_component(eid, std::forward<Cs>(cs)), ...);
            return eid;
        }


        /// ### Access to components

        /// #### Add a component to the entity
        template<typename C>
        auto add_component(
                entity_id &eid,
                C &&component,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            static constexpr auto ci = maybe_component_index<C>();
            if constexpr (ci) {
                assert_entities();
                std::get<ci.value()>(components).at(eid.id) =
                        std::move(component);
                eid->components[*entities_storage_index] |= (1 << ci.value());
                return component_proxy<C, Components...>{*this, eid};
            } else {
                detail::throw_component_type_not_valid(loc);
            }
        }
        /// #### Remove a component from the entity
        template<typename C>
        void remove_component(entity_id &eid) {
            static constexpr auto ci = maybe_component_index<C>();
            if constexpr (ci) {
                assert_entities();
                std::get<ci.value()>(components).at(eid.id).reset();
                eid->components[*entities_storage_index] &= ~(1 << ci.value());
            } else {
                detail::throw_component_type_not_valid();
            }
        }

        /// #### Check if a component is present
        template<typename C>
        [[nodiscard]] bool has_component(entity_id const &eid) const {
            static constexpr auto ci = component_index<C>();
            assert_entities();
            return eid->components[*entities_storage_index] bitand (1 << ci);
        }

        /// #### Retrieve a component
        template<typename C>
        [[nodiscard]] C &get_component(
                entity_id &eid,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            static constexpr auto ci = component_index<C>();
            assert_entities();
            if (has_component<C>(eid)) {
                return std::get<ci>(components).at(eid.id).value();
            } else {
                detail::throw_component_not_present(loc);
            }
        }


        /// ### Coroutine awaitable triggered when a component is destroyed
        // template<typename C>
        // auto on_destroy(entity_id &eid) {
        //     constexpr auto ci = component_index<C>();
        //     return eid->co_on_destroy(1 << ci);
        // }


        /// ### Iterate over entities with the requested components
        template<typename T>
        struct types : public types<decltype(&T::operator())> {};
        template<typename R, typename C, typename... Cs>
        struct types<R (C::*)(entity_id, Cs...) const> {
            static constexpr mask_type mask = {[]() {
                constexpr std::array<std::size_t, sizeof...(Cs)> args = {
                        *maybe_component_index<std::remove_const_t<
                                std::remove_reference_t<Cs>>>()...,
                };
                mask_type m{};
                for (auto const &i : args) { m |= (1 << i); }
                return m;
            }()};

            template<typename L>
            R invoke(entity_id &&eid, L const &l, storage &s) {
                return l(
                        eid,
                        s.get_component<std::remove_const_t<
                                std::remove_reference_t<Cs>>>(eid)...);
            }
        };
        template<typename L>
        void iterate(L &&lambda) {
            assert_entities();
            types<L> traits;
            for (std::size_t idx{}; idx < std::get<0>(components).size();
                 ++idx) {
                detail::entity &e{entities->entity(idx)};
                if (e.components[*entities_storage_index] bitand traits.mask) {
                    traits.invoke(entity_id{entities, idx}, lambda, *this);
                }
            }
        }

      private:
        /**
         * TODO We should use the mask that's already stored in the entities to
         * determine which entries contain the component, then we can use the
         * raw memory type here instead.
         */
        template<typename C>
        using component_storage_type =
                std::vector<felspar::memory::holding_pen<C>>;
        using storage_type = std::tuple<component_storage_type<Components>...>;

        /// The index of this storage
        void set_entities_storage_index(
                detail::entity_lookup *el, std::size_t const si) {
            entities = el;
            entities_storage_index = si;
        }
        detail::entity_lookup *entities = nullptr;
        std::optional<std::size_t> entities_storage_index;

        /// TODO Use a ring so we can chain (like a rope data structure)
        // struct ring {
        //     ring(std::size_t);
        //
        //     storage_type storage;
        // };
        // std::unordered_map<std::size_t, ring> chain;
        storage_type components;
    };


    template<typename Component, typename... Components>
    inline auto component_proxy<Component, Components...>::get() const
            -> component_type * {
        static constexpr auto ci =
                storage_type::template component_index<Component>();
        store.assert_entities();
        if (store.template has_component<Component>(eid)) {
            return &std::get<ci>(store.components).at(eid.id).value();
        } else {
            return nullptr;
        }
    }


}
