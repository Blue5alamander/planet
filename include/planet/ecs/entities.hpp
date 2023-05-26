#pragma once


#include <planet/ecs/entity_lookup.hpp>
#include <planet/ecs/storage_base.hpp>

#include <felspar/exceptions.hpp>

#include <tuple>


namespace planet::ecs {


    /// ## Entities
    /**
     * The game should create a single instance of this type giving it the
     * individual entity storages so that they can be aggregated and ensure that
     * all entities have access to all components so that systems can iterate
     * them properly.
     */
    template<typename... Storages>
    class entities final :
    public detail::entity_lookup,
            public base_entities<Storages>... {
        std::vector<detail::entity> e_slots;
        std::tuple<Storages &...> stores;
        using indexes = std::index_sequence_for<Storages...>;

      public:
        /// ### Construction
        entities(Storages &...s) : stores{s...} {
            [this]<std::size_t... Is>(std::index_sequence<Is...>) {
                (std::get<Is>(stores).set_entities_storage_index(this, Is),
                 ...);
            }
            (indexes{});
            /**
             * We want entity ID zero to be invalid so that a default
             * constructed `entity_id` can take on an invalid value. We can do
             * this by just creating an entity and throwing away the `entity_id`
             * which will have had the value zero.
             *
             * Because it has been thrown away this first entity will never have
             * any components, and thus it is safe to dereference or iterate
             * over as well.
             */
            auto discard [[maybe_unused]] = create();
        }

        /// #### Not movable, not copyable
        entities(entities &&) = delete;
        entities(entities const &) = delete;
        entities &operator=(entities &&) = delete;
        entities &operator=(entities const &) = delete;


        /// ### The number of entity storages that are in use
        static constexpr std::size_t store_count = sizeof...(Storages);


        /// ### Create and return new entity
        [[nodiscard]] entity_id create() override {
            /// TODO Search for a free slot in e_slots
            e_slots.emplace_back(store_count);

            [this]<std::size_t... Is>(std::index_sequence<Is...>) {
                (std::get<Is>(stores).emplace_back(), ...);
            }
            (indexes{});

            return entity_id{this, e_slots.size() - 1};
        }
        template<typename... Components>
        [[nodiscard]] entity_id create(Components &&...components) {
            auto eid = create();
            (add_component(eid, std::forward<Components>(components)), ...);
            return eid;
        }


        /// ### Return the entity
        detail::entity &entity(std::size_t const eid) override {
            return e_slots.at(eid);
        }
        detail::entity const &entity(std::size_t const eid) const override {
            return e_slots.at(eid);
        }


        /// ### Add a component
        template<typename Component>
        auto add_component(entity_id &eid, Component &&component) {
            static constexpr auto storage =
                    get_storage_index_for_type<Component>();
            return std::get<storage>(stores).add_component(
                    eid, std::forward<Component>(component));
        }


        /// ### Remove a component
        template<typename Component>
        auto remove_component(entity_id &eid) {
            static constexpr auto storage =
                    get_storage_index_for_type<Component>();
            return std::get<storage>(stores)
                    .template remove_component<Component>(eid);
        }


        /// ### Does the component exist
        template<typename Component>
        [[nodiscard]] bool has_component(entity_id &eid) {
            static constexpr auto storage =
                    get_storage_index_for_type<Component>();
            auto &store = std::get<storage>(stores);
            return store.template has_component<Component>(eid);
        }


        /// ### Fetch a component
        template<typename Component>
        [[nodiscard]] Component &get_component(
                entity_id &eid,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            static constexpr auto storage =
                    get_storage_index_for_type<Component>();
            auto &store = std::get<storage>(stores);
            return store.template get_component<Component>(eid, loc);
        }


        /// ### Iterate over components
        template<typename Lambda>
        auto iterate(Lambda &&lambda) {
            for (std::size_t idx{1}; idx < e_slots.size(); ++idx) {
                entity_id eid{this, idx};
                using ltt = lambda_tuple_type<Lambda>;
                if (ltt::has_args(this, eid)) {
                    auto args{ltt::get_args(this, eid)};
                    std::apply(lambda, args);
                }
            }
        }


      private:
        void destroy(std::size_t const id) override {
            [ this, id ]<std::size_t... Is>(std::index_sequence<Is...>) {
                (std::get<Is>(stores).destroy(id), ...);
            }
            (indexes{});
        }

        template<typename T>
        struct lambda_tuple_type :
        public lambda_tuple_type<decltype(&T::operator())> {};

        template<typename R, typename C, typename... Cs>
        struct lambda_tuple_type<R (C::*)(entity_id, Cs...) const> {
            using tuple_type = std::tuple<entity_id &, Cs...>;

            static bool has_args(entities *s, entity_id &eid) {
                return (s->has_component<std::remove_const_t<
                                std::remove_reference_t<Cs>>>(eid)
                        && ...);
            }
            static tuple_type get_args(entities *s, entity_id &eid) {
                return {eid,
                        s->get_component<std::remove_const_t<
                                std::remove_reference_t<Cs>>>(eid)...};
            }
        };

        template<typename Component>
        static constexpr std::size_t get_storage_index_for_type() {
            std::array type_indexes{
                    Storages::template maybe_component_index<Component>()...};
            for (std::size_t idx{}; auto const &ti : type_indexes) {
                if (ti.has_value()) { return idx; }
                ++idx;
            }
            throw felspar::stdexcept::logic_error{
                    "Type not found in any storage"};
        }
    };


}
