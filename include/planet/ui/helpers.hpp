#pragma once


#include <planet/affine/rectangle2d.hpp>
#include <planet/ui/constrained.hpp>
#include <planet/ui/forward.hpp>

#include <felspar/exceptions.hpp>

#include <array>
#include <span>
#include <tuple>


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


    namespace detail {
        template<typename Renderer, typename Item>
        void draw(Renderer &r, Item &item, felspar::source_location const &loc)
            requires requires {
                         std::declval<Item>().draw(
                                 std::declval<Renderer>(),
                                 std::declval<felspar::source_location>());
                     }
        {
            item.draw(r, loc);
        }
        template<typename Renderer, typename Item>
        void draw(Renderer &r, Item &item, felspar::source_location const &loc)
            requires(not requires {
                             std::declval<Item>().draw(
                                     std::declval<Renderer>(),
                                     std::declval<felspar::source_location>());
                         })
        {
            try {
                item.draw(r);
            } catch (felspar::stdexcept::logic_error const &e) {
                throw felspar::stdexcept::logic_error{e.what(), loc};
            }
        }
    }
    template<typename Renderer, typename... Pack, std::size_t... I>
    inline void draw_items(
            Renderer &r,
            std::tuple<Pack...> &items,
            felspar::source_location const &loc,
            std::index_sequence<I...>) {
        (detail::draw(r, std::get<I>(items), loc), ...);
    }
    template<typename Renderer, typename... Pack>
    inline void draw_items(
            Renderer &r,
            std::tuple<Pack...> &items,
            felspar::source_location const &loc =
                    felspar::source_location::current()) {
        draw_items(r, items, loc, std::make_index_sequence<sizeof...(Pack)>{});
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
