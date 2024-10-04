#pragma once


#include <felspar/exceptions/logic_error.hpp>

#include <optional>
#include <tuple>
#include <utility>


namespace planet {


    /// ## Helpers for `std::tuple`


    namespace detail::tuple {
        template<typename V>
        inline std::optional<V>
                operator or(std::optional<V> const l, std::optional<V> const r) {
            if (not l) {
                return r;
            } else {
                if (r) {
                    throw felspar::stdexcept::logic_error{
                            "Too many optionals with values to collapse them"};
                } else {
                    return l;
                }
            }
        }
        template<
                typename Return,
                std::size_t Index,
                typename Lambda,
                typename... Items>
        inline std::optional<Return> exec_return(
                std::tuple<Items...> &items, std::size_t const i, Lambda &fn) {
            if (Index == i) {
                return fn(std::get<Index>(items));
            } else {
                return {};
            }
        }
        template<std::size_t Index, typename Lambda, typename... Items>
        inline void exec_void(
                std::tuple<Items...> &items, std::size_t const i, Lambda &fn) {
            if (Index == i) { fn(std::get<Index>(items)); }
        }
    }


    /// ### Execute a lambda on the `i`ᵗʰ value

    /// #### Returning the value
    template<typename Lambda, typename... Items>
    inline auto return_exec_ith_item(
            std::tuple<Items...> &items, std::size_t const i, Lambda &&fn) {
        using return_type = decltype(fn(std::get<0>(items)));
        if (i >= sizeof...(Items)) {
            throw felspar::stdexcept::logic_error{"Item index too high"};
        } else {
            auto const match = [&]<std::size_t... I>(std::index_sequence<I...>)
                    -> std::optional<return_type> {
                using namespace detail::tuple;
                return (exec_return<return_type, I>(items, i, fn) or ...);
            };
            return match(std::make_index_sequence<sizeof...(Items)>{}).value();
        }
    }
    /// #### Without returning a value
    template<typename Lambda, typename... Items>
    inline void exec_ith_item(
            std::tuple<Items...> &items, std::size_t const i, Lambda &&fn) {
        if (i >= sizeof...(Items)) {
            throw felspar::stdexcept::logic_error{"Item index too high"};
        } else {
            auto const match =
                    [&]<std::size_t... I>(std::index_sequence<I...>) {
                        (detail::tuple::exec_void<I>(items, i, fn), ...);
                    };
            match(std::make_index_sequence<sizeof...(Items)>{});
        }
    }


    /// ### Execute on every member
    template<typename Lambda, typename... Items>
    inline void exec_all(std::tuple<Items...> &items, Lambda &&fn) {
        auto const match = [&]<std::size_t... I>(std::index_sequence<I...>) {
            (fn(std::get<I>(items)), ...);
        };
        match(std::make_index_sequence<sizeof...(Items)>{});
    }


}
