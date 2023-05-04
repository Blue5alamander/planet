#pragma once


#include <array>
#include <span>
#include <tuple>

#include <planet/ui/constrained.hpp>
#include <planet/ui/forward.hpp>


namespace planet::ui {


    /// ## Reflow helpers
    /**
     * Helpers for transitioning between the old extents layout system and the
     * new reflow one
     */
    template<typename W, typename V = void>
    concept reflowable_concept =
            requires(W w) {
                typename W::constrained_type;
                { w.reflow(std::declval<typename W::constrained_type>()) };
                { w.size() } -> std::same_as<affine::extents2d const &>;
            };


    /// Legacy reflow using extents
    template<typename W, typename T>
    inline auto reflow(W &widget, constrained2d<T> const &constraint) {
        return constrained2d<T>{widget.extents(constraint.extents())};
    }
    template<typename W>
    inline auto reflow(W &widget, affine::extents2d const &ex) {
        return widget.extents(ex);
    }

    /// Using reflow
    template<reflowable_concept W>
    inline auto
            reflow(W &widget, typename W::constrained_type const &constraint) {
        return widget.reflow(constraint);
    }
    template<reflowable_concept W>
    inline auto reflow(W &widget, affine::extents2d const &ex) {
        return widget.reflow(typename W::constrained_type{ex}).extents();
    }


    /// ## Helpers for tuples
    namespace detail {
        template<typename... Pack, std::size_t... I, typename Ex>
        inline auto item_sizes_helper(
                std::tuple<Pack...> &items,
                [[maybe_unused]] Ex const &outer,
                std::index_sequence<I...>) {
            return std::array<Ex, sizeof...(Pack)>{
                    reflow(std::get<I>(items), outer)...,
            };
        }
    }
    template<typename... Pack, std::size_t... I, typename Ex>
    inline auto item_sizes(std::tuple<Pack...> &items, Ex const &outer) {
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
