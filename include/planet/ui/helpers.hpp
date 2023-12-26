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
        template<typename Item>
        void draw(Item &item) {
            item.draw();
        }
    }
    /// TODO Should move to detail namespace?
    template<typename... Pack, std::size_t... I>
    inline void
            draw_items(std::tuple<Pack...> &items, std::index_sequence<I...>) {
        (detail::draw(std::get<I>(items)), ...);
    }
    template<typename... Pack>
    inline void draw_items(std::tuple<Pack...> &items) {
        draw_items(items, std::make_index_sequence<sizeof...(Pack)>{});
    }


}
