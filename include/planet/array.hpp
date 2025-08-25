#pragma once


#include <array>
#include <concepts>
#include <utility>


namespace planet {


    /// ## `array_of` -- Create an array of `N` values

    /// ### By invoking a lambda
    template<std::size_t N, std::invocable<std::size_t> V>
    inline auto array_of(V &&v) {
        return [&]<std::size_t... Indices>(
                       std::integer_sequence<std::size_t, Indices...>) {
            return std::array{v(Indices)...};
        }(std::make_index_sequence<N>{});
    }
    template<std::size_t N, std::invocable<> V>
    inline auto array_of(V &&v) {
        return array_of<N>([&](std::size_t) { return v(); });
    }

    /// ### By copying an object
    template<std::size_t N, std::copyable V>
    inline auto array_of(V const &v) {
        return array_of<N>([&](std::size_t) { return v; });
    }


}
