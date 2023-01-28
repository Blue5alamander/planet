#pragma once


#include <array>
#include <span>
#include <tuple>


namespace planet::ui {


    namespace detail {
        template<typename... Pack, std::size_t... I>
        inline auto item_sizes_helper(
                std::tuple<Pack...> const &items,
                affine::extents2d const outer,
                std::index_sequence<I...>) {
            return std::array<affine::extents2d, sizeof...(Pack)>{
                    std::get<I>(items).extents(outer)...,
            };
        }
    }
    template<typename... Pack, std::size_t... I>
    inline auto item_sizes(
            std::tuple<Pack...> const &items, affine::extents2d const outer) {
        return detail::item_sizes_helper(
                items, outer, std::make_index_sequence<sizeof...(Pack)>{});
    }


    template<typename Target, typename... Pack, std::size_t... I>
    void draw_items_within(
            Target &t,
            std::tuple<Pack...> &items,
            std::span<affine::rectangle2d> const locations,
            std::index_sequence<I...>) {
        (std::get<I>(items).draw_within(t, locations[I]), ...);
    }


}
