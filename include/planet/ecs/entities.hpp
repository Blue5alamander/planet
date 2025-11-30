#pragma once


#include <planet/ecs/entity_lookup.hpp>
#include <planet/ecs/exceptions.hpp>
#include <planet/ecs/storage_base.hpp>
#include <planet/telemetry/counter.hpp>
#include <planet/telemetry/id.hpp>

#include <tuple>


namespace planet::ecs {


    namespace detail {
        void count_create_entity() noexcept;
        void count_recycled_entity() noexcept;
        void count_destroy_entity() noexcept;
    }


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
        struct e_slot {
            /// Entity contains reference counts and APIs
            detail::entity entity{};
            /// Bitset of the components that this entity uses in each of the
            /// storages
            std::array<mask_type, sizeof...(Storages)> masks{};
            /// Store an optional id that can be used to the latest incarnation
            /// of this entity
            std::optional<telemetry::id> id;
        };
        std::vector<e_slot> e_slots;
        std::tuple<Storages &...> stores;
        using indexes = std::index_sequence_for<Storages...>;

        std::vector<std::size_t> free_list;

        /// ### Keep entity ID zero alive
        /**
         * We want entity ID zero to be invalid so that a default constructed
         * `entity_id` can take on an invalid value. We can do this by just
         * creating an entity and never use it.
         *
         * By denying access to it this first entity will never have any
         * components, and thus it is safe to dereference or iterate over as
         * well.
         */
        entity_id zero;

      public:
        /// ### Construction
        entities(Storages &...s) : stores{s...} {
            [this]<std::size_t... Is>(std::index_sequence<Is...>) {
                (std::get<Is>(stores).set_entities_storage_index(this, Is),
                 ...);
            }(indexes{});
            zero = create();
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
            if (free_list.empty()) {
                e_slots.emplace_back();

                [this]<std::size_t... Is>(std::index_sequence<Is...>) {
                    (std::get<Is>(stores).emplace_back(), ...);
                }(indexes{});

                detail::count_create_entity();

                return entity_id{
                        this, e_slots.size() - 1,
                        ++e_slots.back().entity.generation};
            } else {
                auto const id = free_list.back();
                free_list.pop_back();

                auto &e = e_slots.at(id);
                ++e.entity.generation;
                e.id.reset();
                detail::count_recycled_entity();

                return entity_id{this, id, e.entity.generation};
            }
        }
        [[nodiscard]] entity_id create(std::string n) override {
            auto const eid = create();
            n += "__id_";
            n += std::to_string(eid.id());
            e_slots.at(eid.id()).id.emplace(std::move(n));
            return eid;
        }
        template<typename... Components>
        [[nodiscard]] entity_id create(Components &&...components) {
            auto eid = create();
            (add_component(eid, std::forward<Components>(components)), ...);
            return eid;
        }
        template<typename... Components>
        [[nodiscard]] entity_id
                create(std::string n, Components &&...components) {
            auto eid = create(std::move(n));
            (add_component(eid, std::forward<Components>(components)), ...);
            return eid;
        }
        template<typename... Components>
        [[nodiscard]] entity_id
                create(std::string_view const n, Components &&...components) {
            return create(
                    std::string{n}, std::forward<Components>(components)...);
        }
        template<typename... Components>
        [[nodiscard]] entity_id
                create(char const *const n, Components &&...components) {
            return create(
                    std::string{n}, std::forward<Components>(components)...);
        }


        /// ### Return the name (if any)
        std::optional<telemetry::id> const &
                id(std::size_t const eid) const override {
            return e_slots.at(eid).id;
        }

        /// ### Return the entity
        detail::entity &
                entity(std::size_t const eid,
                       std::source_location const & =
                               std::source_location::current()) override {
            return e_slots.at(eid).entity;
        }
        detail::entity &
                entity(std::size_t const eid,
                       std::size_t const gen,
                       std::source_location const &loc =
                               std::source_location::current()) override {
            auto &e = e_slots.at(eid).entity;
            assert_correct_generation(eid, gen, e.generation, loc);
            return e;
        }
        detail::entity const &
                entity(std::size_t const eid,
                       std::size_t const gen,
                       std::source_location const &loc =
                               std::source_location::current()) const override {
            auto const &e = e_slots.at(eid).entity;
            assert_correct_generation(eid, gen, e.generation, loc);
            return e;
        }
        detail::entity *maybe_entity(
                std::size_t const eid,
                std::size_t const gen,
                std::source_location const & =
                        std::source_location::current()) override {
            auto &e = e_slots.at(eid).entity;
            if (e.generation != gen) {
                return nullptr;
            } else {
                return &e;
            }
        }


        /// ### Check if an entity ID is still a valid entity
        bool is_valid(entity_id const &eid) {
            return e_slots.at(eid.m_id).entity.generation == eid.generation;
        }


        /// ### Add a component
        template<typename Component>
        auto add_component(entity_id &eid, Component &&component) {
            using ctype = std::remove_cvref_t<Component>;
            static constexpr auto storage = get_storage_index_for_type<ctype>();
            return std::get<storage>(stores).add_component(
                    eid, std::forward<Component>(component));
        }
        template<typename... Cs>
        auto add_components(entity_id &eid, Cs &&...cs) {
            return std::tuple{add_component<Cs>(eid, std::forward<Cs>(cs))...};
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
                std::source_location const &loc =
                        std::source_location::current())
        /**
         * Returns a component if it exists. If there is no component of the
         * requested type for the entity then an exception will be thrown.
         */
        {
            static constexpr auto storage =
                    get_storage_index_for_type<Component>();
            auto &store = std::get<storage>(stores);
            return store.template get_component<Component>(eid, loc);
        }
        template<typename Component>
        [[nodiscard]] Component *maybe_get_component(
                entity_id &eid,
                std::source_location const &loc =
                        std::source_location::current())
        /**
         * Returns a pointer to the component if it exists for the entity. If
         * none exists then return `nullptr` instead.
         */
        {
            static constexpr auto storage =
                    get_storage_index_for_type<Component>();
            auto &store = std::get<storage>(stores);
            return store.template maybe_get_component<Component>(eid, loc);
        }
        template<typename Component, std::invocable<> Lambda = Component (*)()>
        [[nodiscard]] Component &always_get_component(
                entity_id &eid,
                Lambda &&lambda = []() { return Component{}; },
                std::source_location const &loc =
                        std::source_location::current())
        /**
         * Return the pre-existing component if it exists, but if it doesn't
         * then use the `lambda` to create the component and then return it. The
         * default lambda is only suitable when the required component can be
         * default constructed.
         */
        {
            static constexpr auto storage =
                    get_storage_index_for_type<Component>();
            auto &store = std::get<storage>(stores);
            Component *c =
                    store.template maybe_get_component<Component>(eid, loc);
            if (c) {
                return *c;
            } else {
                /// TODO It's likely more efficient to implement this on the
                /// `storage`
                return std::get<storage>(stores)
                        .template add_component<Component>(eid, lambda())();
            }
        }


        /// ### Get a component proxy
        template<typename Component>
        [[nodiscard]] auto get_proxy_for(entity_id const &eid) {
            static constexpr auto storage =
                    get_storage_index_for_type<Component>();
            auto &store = std::get<storage>(stores);
            return store.template get_proxy_for<Component>(eid);
        }


        /// ### Iterate over components
        template<typename Lambda>
        auto iterate(Lambda &&lambda)
        /**
         * The `lambda` must take an `entity_id` as the first argument.
         * Subsequent arguments are the components that are required. A
         * component can be taken as a reference (or const reference), in which
         * case it must exist on the entity, otherwise the entity is skipped
         * over. A component argument can also be taken as a pointer to a
         * component. In this case it is optional and if the entity doesn't have
         * the component then `nullptr` is passed as the value.
         *
         * This means that the lambda will only be called for all entities that
         * have all of the components that are passed by reference.
         *
         * The result of the lambda is not used. Any side-effects should be
         * effected in the body of the lambda.
         */
        {
            for (std::size_t idx{1}; idx < e_slots.size(); ++idx) {
                if (e_slots[idx].entity.strong_count) {
                    entity_id eid{this, idx, e_slots[idx].entity.generation};
                    using ltt = lambda_tuple_type<Lambda>;
                    if (ltt::has_args(this, eid)) {
                        auto args{ltt::get_args(this, eid)};
                        std::apply(lambda, args);
                    }
                }
            }
        }


        /// ### Kill a single entity
        void
                kill(entity_id const &eid,
                     std::source_location const & =
                             std::source_location::current()) override {
            auto *entity = maybe_entity(eid.id(), eid.generation);
            if (entity) { destroy(eid, *entity); }
        }

        /// ### Clear all entities and components
        void clear() {
            for (std::size_t idx{1}; idx < e_slots.size(); ++idx) {
                auto &entity = e_slots[idx].entity;
                if (entity.strong_count) {
                    destroy(entity_id{this, idx, entity.generation}, entity);
                }
            }
        }


      private:
        static void assert_correct_generation(
                std::size_t const eid,
                std::size_t const expected_gen,
                std::size_t const actual_gen,
                std::source_location const &loc) {
            if (expected_gen != actual_gen) {
                detail::throw_wrong_generation(
                        eid, expected_gen, actual_gen, loc);
            }
        }

        mask_type &mask_for(
                std::size_t const storage_index,
                std::size_t const eid,
                std::size_t const gen,
                std::source_location const &loc) override {
            auto &e = e_slots.at(eid);
            assert_correct_generation(eid, gen, e.entity.generation, loc);
            return e.masks[storage_index];
        }
        mask_type mask_for(
                std::size_t const storage_index,
                std::size_t const eid,
                std::size_t const gen,
                std::source_location const &loc) const override {
            auto const &e = e_slots.at(eid);
            assert_correct_generation(eid, gen, e.entity.generation, loc);
            return e.masks[storage_index];
        }


        void
                acquire(entity_id const &eid,
                        std::source_location const &) override {
            auto &entity = e_slots[eid.m_id].entity;
            if (entity.generation == eid.generation) {
                entity.increment_strong();
            }
        }
        void release(entity_id const &eid) noexcept override {
            auto &entity = e_slots[eid.m_id].entity;
            if (entity.generation == eid.generation) {
                if (entity.decrement_strong() == 0) { destroy(eid); }
            }
        }

        void destroy(entity_id const &eid, detail::entity &entity) noexcept {
            ++entity.generation;
            entity.strong_count = {};
            auto &c = e_slots[eid.id()].masks;
            std::fill(c.begin(), c.end(), 0);
            [this, &eid]<std::size_t... Is>(std::index_sequence<Is...>) {
                (std::get<Is>(stores).destroy(eid), ...);
            }(indexes{});
            detail::count_destroy_entity();
            free_list.push_back(eid.id());
        }
        void destroy(entity_id const &eid) override {
            auto &entity = e_slots[eid.m_id].entity;
            if (entity.generation == eid.generation) { destroy(eid, entity); }
        }


        template<typename T>
        struct lambda_tuple_type :
        public lambda_tuple_type<decltype(&T::operator())> {};

        template<typename R, typename C, typename... Cs>
        struct lambda_tuple_type<R (C::*)(entity_id, Cs...) const> {
            using tuple_type = std::tuple<entity_id &, Cs...>;

            static bool has_args(entities *s, entity_id &eid) {
                return (has_arg<Cs>(s, eid) && ...);
            }
            template<typename IC>
            static bool has_arg(entities *s, entity_id &eid) {
                if constexpr (has_storage_index_for_type<std::remove_const_t<
                                      std::remove_reference_t<IC>>>()) {
                    return s->has_component<
                            std::remove_const_t<std::remove_reference_t<IC>>>(
                            eid);
                } else {
                    return std::is_pointer_v<IC>
                            and has_storage_index_for_type<std::remove_const_t<
                                    std::remove_pointer_t<IC>>>();
                }
            }
            static tuple_type get_args(entities *s, entity_id &eid) {
                return {eid, get_component<Cs>(s, eid)...};
            }
            template<typename IC>
            static decltype(auto) get_component(entities *s, entity_id &eid) {
                if constexpr (has_storage_index_for_type<std::remove_const_t<
                                      std::remove_reference_t<IC>>>()) {
                    return s->get_component<
                            std::remove_const_t<std::remove_reference_t<IC>>>(
                            eid);
                } else {
                    return s->maybe_get_component<
                            std::remove_const_t<std::remove_pointer_t<IC>>>(
                            eid);
                }
            }
        };

        template<typename Component>
        static constexpr std::size_t has_storage_index_for_type() {
            static constexpr std::array type_indexes{
                    Storages::template maybe_component_index<Component>()...};
            for (auto const &ti : type_indexes) {
                if (ti.has_value()) { return true; }
            }
            return false;
        }
        template<typename Component>
        static constexpr std::size_t get_storage_index_for_type() {
            static constexpr std::array type_indexes{
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
