#pragma once


#include <planet/affine/rectangle2d.hpp>
#include <planet/ui/concepts.hpp>
#include <planet/ui/constrained.hpp>
#include <planet/ui/forward.hpp>

#include <felspar/exceptions.hpp>

#include <array>
#include <span>
#include <tuple>


namespace planet::ui {


    /// ## Helpers for tuples


    /// ### `draw`
    namespace detail {
        template<typename Item>
        void draw(Item &item) {
            item.draw();
        }
    }
    template<typename... Pack, std::size_t... I>
    inline void
            draw_items(std::tuple<Pack...> &items, std::index_sequence<I...>) {
        (detail::draw(std::get<I>(items)), ...);
    }


    /// ### Visibility
    namespace detail {
        template<typename Item>
        void visible(Item &item, bool const v)
            requires visibility<Item>
        {
            item.visible(v);
        }
        template<typename Item>
        void visible(Item &, bool)
            requires(not visibility<Item>)
        {}
    }
    template<typename... Pack, std::size_t... I>
    inline void visible_items(
            std::tuple<Pack...> &items,
            bool const v,
            std::index_sequence<I...>) {
        (detail::visible(std::get<I>(items), v), ...);
    }

    namespace detail {
        template<typename Item>
        bool is_visible(Item const &item) noexcept
            requires visibility<Item>
        {
            return item.is_visible();
        }
        template<typename Item>
        bool is_visible(Item const &) noexcept
            requires(not visibility<Item>)
        {
            return true;
        }
    }
    template<typename... Pack, std::size_t... I>
    inline bool is_visible_items(
            std::tuple<Pack...> const &items,
            std::index_sequence<I...>) noexcept {
        return (... or detail::is_visible(std::get<I>(items)));
    }


}
