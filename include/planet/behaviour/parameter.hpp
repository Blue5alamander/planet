#pragma once


#include <planet/behaviour/type.hpp>

#include <concepts>
#include <typeinfo>


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


        char const *name;
        behaviour::type type;


        constexpr parameter(char const *n)
        : name{n}, type{&typeid(argument_type)} {}


        constexpr static bool is_constant() noexcept {
            return std::same_as<argument_type, argument_type const>;
        }
    };


    /// ## Concepts describing parameter compatibility
    /// TODO Handle const promotion of mutable arguments
    template<typename P, typename A>
    concept compatible_parameters = std::same_as<P, typename A::argument_type>;


}
