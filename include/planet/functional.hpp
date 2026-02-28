#pragma once


#include <concepts>
#include <span>
#include <utility>


namespace planet {


    template<typename T>
    concept spannable = requires(T &&obj) {
        std::span{std::forward<T>(obj)};
    } and[]<typename U = T>() {
        using Span = decltype(std::span{std::declval<U>()});
        return not std::same_as<std::remove_cvref_t<T>, Span>;
    }
    ();


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
    constexpr inline auto by_index(std::span<T, N> const s, Lambda &&v) {
        for (std::size_t index{}; index < s.size(); ++index) {
            v(index, s[index]);
        }
    }

    template<
            spannable T,
            std::invocable<
                    std::size_t,
                    typename decltype(std::span{std::declval<
                            std::remove_cvref_t<T> &>()})::element_type &> Lambda>
    constexpr inline auto by_index(T &&a, Lambda &&v) {
        by_index(std::span{std::forward<T>(a)}, std::forward<Lambda>(v));
    }
    template<
            spannable T,
            std::regular_invocable<
                    std::size_t,
                    typename decltype(std::span{std::declval<
                            std::remove_cvref_t<T> &>()})::element_type &> Lambda>
    constexpr inline auto by_index(T const &&a, Lambda &&v) {
        by_index(std::span{std::forward<T>(a)}, std::forward<Lambda>(v));
    }


}
