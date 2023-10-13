#pragma once


#include <planet/ecs/entity_id.hpp>
#include <planet/ecs/exceptions.hpp>


namespace planet::ecs {


    /// ## Component proxy
    /**
     * The proxy allows for terser code to do ECS component look-ups against the
     * storage.
     *
     * It is modelled on a raw pointer (but will be slower) so `const` does not
     * propagate.
     */
    template<typename Component, typename... Components>
    class component_proxy {
        using storage_type = storage<Components...>;
        friend storage_type;

        storage_type &store;
        entity_id eid;

        template<typename C>
        static C *assert_not_null(
                C *p,
                entity_id const &eid,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            if (p == nullptr) {
                detail::throw_component_not_present(eid, typeid(C), loc);
            } else {
                return p;
            }
        }

      public:
        using component_type = Component;


        /// ### Queries
        entity_id const &id() const noexcept { return eid; }

        explicit operator bool() const noexcept { return get() != nullptr; }


        /// ### Fetch the component
        [[nodiscard]] component_type *
                get(felspar::source_location const & =
                            felspar::source_location::current());
        [[nodiscard]] component_type const *
                get(felspar::source_location const &loc =
                            felspar::source_location::current()) const;
        [[nodiscard]] component_type *operator->() {
            return assert_not_null(get(), eid);
        }
        [[nodiscard]] component_type const *operator->() const {
            return assert_not_null(get(), eid);
        }
        [[nodiscard]] component_type &operator*() {
            return *assert_not_null(get(), eid);
        }
        [[nodiscard]] component_type const &operator*() const {
            return *assert_not_null(get(), eid);
        }


        /// ### Destroy the component
        void
                remove(felspar::source_location const & =
                               felspar::source_location::current());


      private:
        /// ### Construction
        component_proxy(storage_type &s, entity_id const &e)
        : store{s}, eid{e} {}
    };


}
