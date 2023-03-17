#pragma once


#include <concepts>


namespace planet::behaviour {


    /// ## A parameter
    /**
     * Performs a key lookup in the context and provides the correct type
     * (always a reference)
     */
    template<typename T>
    struct parameter {
        using pointer_type = std::add_pointer_t<T>;
        using argument_type = std::add_lvalue_reference_t<T>;
        using const_argument_type = std::add_lvalue_reference_t<T const>;

        behaviour::key key;

        constexpr explicit parameter(key::id_type const n) : key{n} {}
    };


    /// ## Concepts describing parameter compatibility
    template<typename P, typename A>
    concept compatible_parameters = std::same_as<P, typename A::argument_type>
            or std::same_as<P, typename A::const_argument_type>;


}
