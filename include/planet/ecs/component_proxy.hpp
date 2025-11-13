#pragma once


#include <planet/ecs/entity_id.hpp>
#include <planet/ecs/exceptions.hpp>


namespace planet::ecs {


    namespace detail {
        template<typename Component>
        struct abstract_lookup {
            virtual void
                    lookup(entity_id const &,
                           Component **,
                           std::source_location const & =
                                   std::source_location::current()) = 0;
            virtual void
                    lookup(entity_id const &,
                           Component const **,
                           std::source_location const & =
                                   std::source_location::current()) const = 0;
            virtual void
                    remove(entity_id &,
                           std::source_location const & =
                                   std::source_location::current()) = 0;
        };
    }


    /// ## Component proxy
    /**
     * The proxy allows for terser code to do ECS component look-ups against the
     * storage.
     */
    template<typename Component>
    class component_proxy {
        detail::abstract_lookup<Component> *store = nullptr;
        entity_id eid = {};

        template<typename C>
        static C *assert_not_null(
                C *p,
                entity_id const &eid,
                std::source_location const &loc =
                        std::source_location::current()) {
            if (p == nullptr) {
                detail::throw_component_not_present(eid, typeid(C), loc);
            } else {
                return p;
            }
        }

      public:
        using component_type = Component;


        /// ### Construction
        component_proxy() {}
        component_proxy(
                detail::abstract_lookup<Component> &s, entity_id const &e)
        : store{&s}, eid{e} {}
        component_proxy(component_proxy const &) = default;
        component_proxy(component_proxy &&) = default;

        component_proxy &operator=(component_proxy const &) = default;
        component_proxy &operator=(component_proxy &&) = default;


        /// ### Queries
        [[nodiscard]] entity_id const &id() const noexcept { return eid; }
        explicit operator bool() const noexcept {
            return store != nullptr and get() != nullptr;
        }


        /// ### Fetch the component
        [[nodiscard]] component_type *
                get(std::source_location const &loc =
                            std::source_location::current()) {
            component_type *component = nullptr;
            store->lookup(eid, &component, loc);
            return component;
        }
        [[nodiscard]] component_type const *
                get(std::source_location const &loc =
                            std::source_location::current()) const {
            component_type const *component = nullptr;
            store->lookup(eid, &component, loc);
            return component;
        }
        [[nodiscard]] component_type &operator()(
                std::source_location const &loc =
                        std::source_location::current()) {
            return *assert_not_null(get(loc), eid, loc);
        }
        [[nodiscard]] component_type const &operator()(
                std::source_location const &loc =
                        std::source_location::current()) const {
            return *assert_not_null(get(loc), eid, loc);
        }


        /// ### Destroy the component
        void
                remove(std::source_location const &loc =
                               std::source_location::current()) {
            store->remove(eid, loc);
        }
    };


}
