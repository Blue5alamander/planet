#pragma once


#include <planet/affine/rectangle2d.hpp>
#include <planet/ui/constrained.hpp>
#include <planet/ui/forward.hpp>

#include <felspar/exceptions.hpp>

#include <array>
#include <span>
#include <tuple>


namespace planet::ui {


    /// ## Helpers for tuples
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


}
