#pragma once


#include <planet/ecs/component_proxy.hpp>
#include <planet/ecs/entities.hpp>
#include <planet/ecs/exceptions.hpp>
#include <planet/ecs/type_index.hpp>

#include <felspar/memory/holding_pen.hpp>
#include <felspar/memory/stable_vector.hpp>


namespace planet::ecs {


    /// ## Holder for all entities
    template<typename... Components>
    class storage final {
        template<typename Component, typename... Cs>
        friend class component_proxy;
        template<typename... Storages>
        friend class entities;

        void assert_entities(
                felspar::source_location const &loc =
                        felspar::source_location::current()) const {
            if (not entities or not entities_storage_index) {
                detail::throw_no_entities_instance(loc);
            }
        }
        void assert_entities(
                entity_id const &eid,
                felspar::source_location const &loc =
                        felspar::source_location::current()) const {
            assert_entities();
            if (not eid) { detail::throw_entity_not_valid(eid, loc); }
        }

      public:
        storage() = default;

        using component_tuple = std::tuple<Components...>;
        static constexpr std::size_t component_count = sizeof...(Components);
        using indexes = std::index_sequence_for<Components...>;

        template<typename C>
        using proxy_for = component_proxy<C, Components...>;

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
            }(indexes{});
        }


        /// ### Create an entity with the desired components
        template<typename... Cs>
        [[nodiscard]] entity_id create(Cs &&...cs) {
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
                assert_entities(eid, loc);
                std::get<ci.value()>(components).at(eid.id()) =
                        std::move(component);
                eid.mask(*entities_storage_index) |= (1 << ci.value());
                return proxy_for<C>{*this, eid};
            } else {
                detail::throw_component_type_not_valid(loc);
            }
        }
        /// #### Provide a component proxy
        template<typename C>
        [[nodiscard]] auto get_proxy_for(entity_id eid) {
            return proxy_for<C>{*this, std::move(eid)};
        }
        /// #### Remove a component from the entity
        template<typename C>
        void remove_component(
                entity_id &eid,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            static constexpr auto ci = maybe_component_index<C>();
            if constexpr (ci) {
                assert_entities(eid);
                eid.mask(*entities_storage_index, loc) &= ~(1 << ci.value());
                std::get<ci.value()>(components).at(eid.id()).reset();
            } else {
                detail::throw_component_type_not_valid(loc);
            }
        }

        /// #### Check if a component is present
        template<typename C>
        [[nodiscard]] bool has_component(
                entity_id const &eid,
                felspar::source_location const &loc =
                        felspar::source_location::current()) const {
            static constexpr auto ci = component_index<C>();
            assert_entities(eid, loc);
            return eid.mask(*entities_storage_index) bitand (1 << ci);
        }

        /// #### Retrieve a component
        template<typename C>
        [[nodiscard]] C *maybe_get_component(
                entity_id const &eid,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            static constexpr auto ci = component_index<C>();
            if (has_component<C>(eid, loc)) {
                return &std::get<ci>(components).at(eid.id()).value(loc);
            } else {
                return nullptr;
            }
        }
        template<typename C>
        [[nodiscard]] C &get_component(
                entity_id const &eid,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            if (auto *p = maybe_get_component<C>(eid, loc); p) {
                return *p;
            } else {
                detail::throw_component_not_present(eid, typeid(C), loc);
            }
        }


        /// ### Coroutine awaitable triggered when a component is destroyed
        // template<typename C>
        // auto on_destroy(entity_id &eid) {
        //     constexpr auto ci = component_index<C>();
        //     return eid->co_on_destroy(1 << ci);
        // }


        /// ### Iterate over entities with the requested components
        template<typename L>
        void
                iterate(L &&lambda,
                        felspar::source_location const &loc =
                                felspar::source_location::current()) {
            assert_entities(loc);
            types<L> traits;
            for (std::size_t idx{}; idx < std::get<0>(components).size();
                 ++idx) {
                detail::entity &e{entities->entity(idx)};
                entity_id eid{entities, idx, e.generation};
                if (e.strong_count
                    and eid.mask(*entities_storage_index) bitand traits.mask) {
                    traits.invoke(std::move(eid), lambda, *this, loc);
                }
            }
        }
        /// #### Iterate over a range of `entity_id`s
        template<typename Range, typename L>
        void
                iterate(Range &&range,
                        L &&lambda,
                        felspar::source_location const &loc =
                                felspar::source_location::current()) {
            assert_entities(loc);
            types<L> traits;
            for (auto &&eid : range) {
                if (eid.mask(*entities_storage_index) bitand traits.mask) {
                    traits.invoke(eid, lambda, *this, loc);
                }
            }
        }


      private:
        void destroy(std::size_t const id) {
            [this, id]<std::size_t... Is>(std::index_sequence<Is...>) {
                (std::get<Is>(components).at(id).reset(), ...);
            }(indexes{});
        }

        /**
         * TODO We should use the mask that's already stored in the entities to
         * determine which entries contain the component, then we can use the
         * raw memory type here instead.
         */
        template<typename C>
        using component_storage_type = felspar::memory::
                stable_vector<felspar::memory::holding_pen<C>, 32>;
        using storage_type = std::tuple<component_storage_type<Components>...>;

        /// The index of this storage
        void set_entities_storage_index(
                detail::entity_lookup *el, std::size_t const si) {
            entities = el;
            entities_storage_index = si;
        }
        detail::entity_lookup *entities = nullptr;
        std::optional<std::size_t> entities_storage_index;
        storage_type components;

        template<typename T>
        struct types : public types<decltype(&T::operator())> {};
        template<typename R, typename C, typename... Cs>
        struct types<R (C::*)(entity_id, Cs...) const> {
            template<typename CsT>
            using ctype_for = std::remove_const_t<std::remove_reference_t<CsT>>;

            static constexpr mask_type mask = {[]() {
                constexpr std::array<std::size_t, sizeof...(Cs)> args = {
                        component_index<ctype_for<Cs>>()...,
                };
                mask_type m{};
                for (auto const &i : args) { m |= (1 << i); }
                return m;
            }()};

            template<typename L>
            R
                    invoke(entity_id const &eid,
                           L const &l,
                           storage &s,
                           felspar::source_location const &loc =
                                   felspar::source_location::current()) {
                return l(eid, s.get_component<ctype_for<Cs>>(eid, loc)...);
            }
        };
    };


    template<typename Component, typename... Components>
    inline auto component_proxy<Component, Components...>::get(
            felspar::source_location const &loc) -> component_type * {
        static constexpr auto ci =
                storage_type::template component_index<Component>();
        if (store.template has_component<Component>(eid, loc)) {
            return &std::get<ci>(store.components).at(eid.id()).value();
        } else {
            return nullptr;
        }
    }
    template<typename Component, typename... Components>
    inline auto component_proxy<Component, Components...>::get(
            felspar::source_location const &loc) const
            -> component_type const * {
        static constexpr auto ci =
                storage_type::template component_index<Component>();
        if (store.template has_component<Component>(eid, loc)) {
            return &std::get<ci>(store.components).at(eid.id()).value();
        } else {
            return nullptr;
        }
    }
    template<typename Component, typename... Components>
    inline void component_proxy<Component, Components...>::remove(
            felspar::source_location const &loc) {
        static constexpr auto ci =
                storage_type::template maybe_component_index<Component>();
        if constexpr (ci) {
            store.assert_entities(eid, loc);
            auto &hp = std::get<ci.value()>(store.components).at(eid.id());
            hp.reset();
            eid.mask(*store.entities_storage_index) &= ~(1 << ci.value());
        } else {
            detail::throw_component_type_not_valid();
        }
    }


}
