#pragma once


#include <planet/ui/helpers.hpp>
#include <planet/ui/layout.hpp>
#include <planet/ui/reflowable.hpp>


namespace planet::ui {


    /// ## Reflowable implementation for tuple based layouts
    template<typename ET, typename... Pack>
    struct pack_reflowable : public reflowable {
        using collection_type = std::tuple<Pack...>;
        collection_type items;
        static constexpr std::size_t item_count = sizeof...(Pack);

        pack_reflowable(collection_type c) : items{std::move(c)} {}

        using layout_type = planet::ui::layout<
                std::array<planet::ui::element<ET>, item_count>>;
        using constrained_type = typename layout_type::constrained_type;
        layout_type elements;

        template<typename Renderer>
        void
                draw(Renderer &r,
                     felspar::source_location const &loc =
                             felspar::source_location::current()) {
            draw_items(r, items, loc);
        }

      private:
        static_assert(item_count > 0, "There must be at least one UI element");

        void move_sub_elements(affine::rectangle2d const &r) {
            move_elements(r, std::make_index_sequence<sizeof...(Pack)>{});
        }
        template<std::size_t... I>
        void move_elements(
                affine::rectangle2d const &r, std::index_sequence<I...>) {
            auto const tl = r.top_left;
            (std::get<I>(items).move_to(
                     {elements.at(I).position->top_left + tl,
                      elements.at(I).position->extents}),
             ...);
        }
    };


}
