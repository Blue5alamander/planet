#pragma once


#include <felspar/exceptions.hpp>

#include <optional>


namespace planet::ecs::detail {


    using type_index = std::optional<std::size_t>;

    template<typename L, typename C>
    inline constexpr type_index same_type_index(std::size_t const index) {
        if constexpr (std::is_same_v<L, C>) {
            return index;
        } else {
            return {};
        }
    }

    inline constexpr type_index
            operator||(type_index const l, type_index const r) {
        if (r and l) {
            throw felspar::stdexcept::logic_error{
                    "Each component must be a unique type"};
        } else if (r and not l) {
            return r;
        } else if (l and not r) {
            return l;
        } else {
            return {};
        }
    }

    template<typename L, typename... Cs, std::size_t... Is>
    inline constexpr std::optional<std::size_t>
            component_index_sequence(std::index_sequence<Is...>) {
        using tt = std::tuple<Cs...>;
        return (same_type_index<L, std::tuple_element_t<Is, tt>>(Is) || ...);
    }


}
