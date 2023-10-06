#pragma once


#include <planet/ecs/entity_id.hpp>

#include <felspar/exceptions.hpp>


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
        static C *assert_not_null(C *p) {
            if (p == nullptr) {
                throw felspar::stdexcept::logic_error{
                        "The component that this proxies does not exist"};
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
                get(felspar::source_location const & =
                            felspar::source_location::current()) const;
        [[nodiscard]] component_type *operator->() {
            return assert_not_null(get());
        }
        [[nodiscard]] component_type const *operator->() const {
            return assert_not_null(get());
        }
        [[nodiscard]] component_type &operator*() {
            return *assert_not_null(get());
        }
        [[nodiscard]] component_type const &operator*() const {
            return *assert_not_null(get());
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
