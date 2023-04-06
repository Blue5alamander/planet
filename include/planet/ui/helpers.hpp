#pragma once


#include <array>
#include <span>
#include <tuple>

#include <planet/ui/constrained.hpp>


namespace planet::ui {


    /// ## Reflow helpers
    /**
     * Helpers for transitioning between the old extents layout system and the
     * new reflow one
     */
    template<typename W>
    concept reflowable =
            requires(W w) {
                typename W::constrained_type;
                { w.reflow(w, std::declval<W::constrained_type>()) };
            };
    template<typename W, typename T>
    inline auto reflow(W &widget, constrained2d<T> const &constraint) {
        return widget.extents(constraint.extents());
    }
    template<reflowable W>
    inline auto
            reflow(W &widget, typename W::constrained_type const &constraint) {
        return widget.reflow(constraint);
    }


    /// ## Helpers for tuples
    namespace detail {
        template<typename... Pack, std::size_t... I>
        inline auto item_sizes_helper(
                std::tuple<Pack...> &items,
                [[maybe_unused]] affine::extents2d const outer,
                std::index_sequence<I...>) {
            return std::array<affine::extents2d, sizeof...(Pack)>{
                    std::get<I>(items).extents(outer)...,
            };
        }
    }
    template<typename... Pack, std::size_t... I>
    inline auto item_sizes(
            std::tuple<Pack...> &items, affine::extents2d const outer) {
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
