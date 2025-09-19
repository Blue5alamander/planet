#pragma once


#include <concepts>
#include <utility>


namespace planet {


    /// ## 'by_index' -- Iterate over array/vector indexes

    /// ### Unary lambda taking the index
    template<std::invocable<std::size_t> Lambda>
    constexpr inline auto by_index(
            std::size_t const start_index,
            std::size_t const max_index,
            Lambda &&lambda) {
        for (std::size_t index{start_index}; index < max_index; ++index) {
            lambda(index);
        }
    }
    template<std::invocable<std::size_t> Lambda>
    constexpr inline auto
            by_index(std::size_t const max_index, Lambda &&lambda) {
        return by_index({}, max_index, std::forward<Lambda>(lambda));
    }


    /// ### Nullary lambda
    template<std::invocable<> Lambda>
    constexpr inline auto by_index(std::size_t const max_index, Lambda &&v) {
        return by_index({}, max_index, [&](std::size_t) { return v(); });
    }


    /// ### Over a `std::span`
    template<typename T, std::size_t N, std::invocable<std::size_t, T &> Lambda>
    constexpr inline auto by_index(std::span<T, N> s, Lambda &&v) {
        for (std::size_t index{}; index < s.size(); ++index) {
            v(index, s[index]);
        }
    }
    template<typename T, std::size_t N, std::invocable<std::size_t, T &> Lambda>
    constexpr inline auto by_index(std::array<T, N> &a, Lambda &&v) {
        by_index(std::span{a}, std::forward<Lambda>(v));
    }
    template<
            typename T,
            std::size_t N,
            std::regular_invocable<std::size_t, T &> Lambda>
    constexpr inline auto by_index(std::array<T, N> const &a, Lambda &&v) {
        by_index(std::span{a}, std::forward<Lambda>(v));
    }


}
