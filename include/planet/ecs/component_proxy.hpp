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
        storage<Components...> &store;
        entity_id eid;

      public:
        using component_type = Component;
        using storage_type = storage<Components...>;


        /// ### Construction
        component_proxy(storage_type &s, entity_id const &e)
        : store{s}, eid{e} {}


        /// ### Fetch the component
        component_type *operator->() const {
            /// TODO Check for nullptr being returned
            return get();
        }
        [[nodiscard]] component_type *get() const;


        /// ### Destroy the component
        void remove();
    };


}
