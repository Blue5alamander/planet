#pragma once


#include <planet/ecs/entity_id.hpp>


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
        friend class storage<Components...>;

        storage<Components...> &store;
        entity_id eid;

      public:
        using component_type = Component;
        using storage_type = storage<Components...>;


        /// ### Fetch the component
        [[nodiscard]] component_type *
                get(felspar::source_location const & =
                            felspar::source_location::current());
        [[nodiscard]] component_type const *
                get(felspar::source_location const & =
                            felspar::source_location::current()) const;
        [[nodiscard]] component_type *operator->() {
            /// TODO Check for nullptr being returned
            return get();
        }
        [[nodiscard]] component_type const *operator->() const {
            /// TODO Check for nullptr being returned
            return get();
        }
        [[nodiscard]] component_type &operator*() {
            /// TODO Check for nullptr being returned
            return *get();
        }
        [[nodiscard]] component_type const &operator*() const {
            /// TODO Check for nullptr being returned
            return *get();
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
