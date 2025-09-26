#pragma once


#include <planet/ecs/component_proxy.hpp>
#include <planet/ecs/entities.hpp>
#include <planet/ecs/exceptions.hpp>
#include <planet/ecs/type_index.hpp>
#include <planet/queue/pmc.hpp>

#include <felspar/memory/holding_pen.hpp>
#include <felspar/memory/stable_vector.hpp>


namespace planet::ecs {


    /// #### Description of the component being removed
    template<typename C>
    struct component_removal {
        entity_id eid;
        C *component;
    };


    namespace detail {
        template<typename Component, typename Storage>
        struct concrete_lookup : public abstract_lookup<Component> {
            void
                    lookup(entity_id const &,
                           Component **,
                           felspar::source_location const &) override;
            void
                    lookup(entity_id const &,
                           Component const **,
                           felspar::source_location const &) const override;
            void remove(entity_id &, felspar::source_location const &) override;
        };
    }


    template<typename Component>
    concept connectable = requires(Component c, entity_id eid) {
        { c.connect(eid) };
    };


    /// ## Holder for all entities
    template<typename... Components>
    class storage final :
    public detail::concrete_lookup<Components, storage<Components...>>... {
        template<typename Component>
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
        using proxy_for = component_proxy<C>;

        template<typename L>
        static constexpr std::optional<std::size_t> maybe_component_index() {
            return detail::component_index_sequence<L, Components...>(
                    indexes{});
        }
        template<typename L>
        static constexpr std::size_t component_index() {
            constexpr auto ci =
                    detail::component_index_sequence<L, Components...>(
                            indexes{});
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
        template<typename... Cs>
        [[nodiscard]] entity_id create(std::string n, Cs &&...cs) {
            assert_entities();
            auto eid = entities->create(std::move(n));
            (add_component(eid, std::forward<Cs>(cs)), ...);
            return eid;
        }
        template<typename... Cs>
        [[nodiscard]] auto create(char const *const n, Cs &&...cs) {
            return create(std::string{n}, std::forward<Cs>(cs)...);
        }


        /// ### Access to components

        /// #### Add a component to the entity
        template<typename C>
        auto add_component(
                entity_id &eid,
                C &&component,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            using ctype = std::remove_cvref_t<C>;
            static constexpr auto ci = component_index<ctype>();
            assert_entities(eid, loc);
            auto &nc =
                    (std::get<ci>(components).at(eid.id()) =
                             std::move(component));
            eid.mask(*entities_storage_index) |= (1 << ci);
            connect_component(eid, *nc);
            return proxy_for<ctype>{*this, eid};
        }
        /// #### Provide a component proxy
        template<typename C>
        [[nodiscard]] auto get_proxy_for(entity_id eid) {
            using ctype = std::remove_cvref_t<C>;
            return proxy_for<ctype>{*this, std::move(eid)};
        }
        /// #### Remove a component from the entity
        template<typename C>
        void remove_component(
                entity_id &eid,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            using ctype = std::remove_cvref_t<C>;
            static constexpr auto ci = maybe_component_index<ctype>();
            if constexpr (ci) {
                assert_entities(eid);
                eid.mask(*entities_storage_index, loc) &= ~(1 << ci.value());
                destroy_component<ci.value()>(eid);
            } else {
                detail::throw_component_type_not_valid(eid, typeid(ctype), loc);
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
        [[nodiscard]] C const *maybe_get_component(
                entity_id const &eid,
                felspar::source_location const &loc =
                        felspar::source_location::current()) const {
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


        /// ### Watching for removal of component

        /// #### Access to the queue which will describe removals
        template<typename C>
        queue::pmc<component_removal<C>> &on_destroy_queue_for() {
            static constexpr auto ci = component_index<C>();
            return std::get<ci>(removal_queues);
        }


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
                    and eid.mask(*entities_storage_index, loc)
                            bitand traits.mask) {
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
            for (entity_id const &eid : range) {
                if (eid.mask(*entities_storage_index, loc) bitand traits.mask) {
                    traits.invoke(eid, lambda, *this, loc);
                }
            }
        }


        /// ### Force kill the entity
        void
                kill(entity_id const &eid,
                     felspar::source_location const &loc =
                             felspar::source_location::current()) {
            assert_entities();
            entities->kill(eid, loc);
        }


      private:
        template<typename C>
        static void connect_component(entity_id const &eid, C &component)
            requires connectable<C>
        {
            component.connect(eid);
        }
        template<typename C>
        static void connect_component(entity_id const &, C &)
            requires(not connectable<C>)
        {}


        template<std::size_t C>
        void destroy_component(entity_id const &eid) {
            auto &chp = std::get<C>(components).at(eid.id());
            if (chp) {
                std::get<C>(removal_queues).push({eid, &*chp});
                chp.reset();
            }
        }
        void destroy(entity_id const &eid) {
            [this, &eid]<std::size_t... Is>(std::index_sequence<Is...>) {
                (this->destroy_component<Is>(eid), ...);
            }(indexes{});
        }

        /// The index of this storage
        void set_entities_storage_index(
                detail::entity_lookup *el, std::size_t const si) {
            entities = el;
            entities_storage_index = si;
        }
        detail::entity_lookup *entities = nullptr;
        std::optional<std::size_t> entities_storage_index;

        /**
         * TODO We should use the mask that's already stored in the entities to
         * determine which entries contain the component, then we can use the
         * raw memory type here instead.
         */
        template<typename C>
        using component_storage_type = felspar::memory::
                stable_vector<felspar::memory::holding_pen<C>, 32>;
        using storage_type = std::tuple<component_storage_type<Components>...>;
        storage_type components;

        template<typename C>
        using removal_queue_type = queue::pmc<component_removal<C>>;
        using removal_queues_type =
                std::tuple<removal_queue_type<Components>...>;
        removal_queues_type removal_queues;

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
                for (auto const &i : args) { m |= (std::uint64_t(1) << i); }
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


    namespace detail {
        template<typename Component, typename Storage>
        void concrete_lookup<Component, Storage>::lookup(
                entity_id const &eid,
                Component **component,
                felspar::source_location const &loc) {
            if (eid) {
                Storage &store = dynamic_cast<Storage &>(*this);
                *component =
                        store.template maybe_get_component<Component>(eid, loc);
            }
        }
        template<typename Component, typename Storage>
        void concrete_lookup<Component, Storage>::lookup(
                entity_id const &eid,
                Component const **component,
                felspar::source_location const &loc) const {
            if (eid) {
                Storage const &store = dynamic_cast<Storage const &>(*this);
                *component =
                        store.template maybe_get_component<Component>(eid, loc);
            }
        }
        template<typename Component, typename Storage>
        void concrete_lookup<Component, Storage>::remove(
                entity_id &eid, felspar::source_location const &loc) {
            Storage &store = dynamic_cast<Storage &>(*this);
            store.template remove_component<Component>(eid, loc);
        }
    }


}
