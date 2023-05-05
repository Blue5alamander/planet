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

        pack_reflowable(
                collection_type c,
                felspar::source_location const &loc =
                        felspar::source_location::current())
        : items{std::move(c)}, created_loc{loc} {}

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
        felspar::source_location created_loc;
        static_assert(item_count > 0, "There must be at least one UI element");

        void move_sub_elements(affine::rectangle2d const &r) {
            move_elements(r, std::make_index_sequence<sizeof...(Pack)>{});
        }
        template<std::size_t... I>
        void move_elements(
                affine::rectangle2d const &r, std::index_sequence<I...>) {
            try {
                auto const tl = r.top_left;
                std::array const pos{elements.at(I).position.value()...};
                (std::get<I>(items).move_to(
                         {pos[I].top_left + tl, pos[I].extents}),
                 ...);
            } catch (std::bad_optional_access const &) {
                throw felspar::stdexcept::logic_error{
                        "Element position has not been set for "
                        "`pack_reflowable` created at",
                        created_loc};
            }
        }
    };


}
