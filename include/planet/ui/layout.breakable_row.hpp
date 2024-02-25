#pragma once


#include <planet/ui/collection.reflowable.hpp>
#include <planet/ui/pack.reflowable.hpp>


namespace planet::ui {


    /// ## Draws the items across multiple lines when needed
    template<typename CT>
    struct breakable_row final : public collection_reflowable<CT, void> {
        using superclass = collection_reflowable<CT, void>;
        using collection_type = typename superclass::collection_type;
        using constrained_type = typename superclass::constrained_type;
        using superclass::elements;
        using superclass::items;


        /// ### Padding between items in the row
        float hpadding = {}, vpadding = {};


        breakable_row(collection_type c, float const hp, float const vp)
        : superclass{"planet::ui::breakable_row<C>", std::move(c)},
          hpadding{hp},
          vpadding{vp} {}
        breakable_row(
                std::string_view const n,
                collection_type c,
                float const hp,
                float const vp)
        : superclass{n, std::move(c)}, hpadding{hp}, vpadding{vp} {}
        breakable_row(collection_type c, float const p)
        : breakable_row{std::move(c), p, p} {}
        breakable_row(std::string_view const n, collection_type c, float const p)
        : breakable_row{n, std::move(c), p, p} {}


      private:
        constrained_type do_reflow(constrained_type const &border) override {
            if (elements.size() != items.size()) { elements.resize_to(items); }
            float row_height = {}, x = {}, y = {}, max_width{};
            for (std::size_t index{}; auto &element : elements) {
                auto const ex = items[index].reflow(border);
                if (x + ex.width.value() > border.width.value()) {
                    x = {};
                    if (y) { y += vpadding; }
                    y += row_height;
                    row_height = {};
                }
                if (x) { x += hpadding; }
                element.position = {affine::point2d{x, y}, ex.extents()};
                row_height = std::max(row_height, ex.height.value());
                x += ex.width.value();
                max_width = std::max(x, max_width);
                ++index;
            }
            float const width = max_width, height = row_height + y;
            /// TODO We could calculate better min/max here
            return constrained_type{width, height};
        }
    };


    template<typename... Pack>
    struct breakable_row<std::tuple<Pack...>> final :
    public pack_reflowable<void, Pack...> {
        using superclass = pack_reflowable<void, Pack...>;
        using collection_type = typename superclass::collection_type;
        using constrained_type = typename superclass::constrained_type;
        using superclass::elements;
        using superclass::item_count;
        using superclass::items;


        /// Padding between items in the row
        float hpadding = {}, vpadding = {};


        explicit breakable_row(collection_type c, float const hp, float const vp)
        : superclass{"planet::ui::breakable_row<std::tuple<Pack...>>", std::move(c)},
          hpadding{hp},
          vpadding{vp} {}
        explicit breakable_row(
                std::string_view const n,
                collection_type c,
                float const hp,
                float const vp,
                felspar::source_location const &loc =
                        felspar::source_location::current())
        : superclass{n, std::move(c), loc}, hpadding{hp}, vpadding{vp} {}
        explicit breakable_row(collection_type c, float const p)
        : breakable_row{std::move(c), p, p} {}
        explicit breakable_row(
                std::string_view const n,
                collection_type c,
                float const p,
                felspar::source_location const &loc =
                        felspar::source_location::current())
        : breakable_row{n, std::move(c), p, p, loc} {}


      private:
        constrained_type do_reflow(constrained_type const &border) override {
            float row_height = {}, x = {}, y = {}, max_width{};
            for (std::size_t index{};
                 auto &ex : superclass::items_reflow(border)) {
                if (x + ex.width.value() > border.width.value()) {
                    x = {};
                    if (y) { y += vpadding; }
                    y += row_height;
                    row_height = {};
                }
                if (x) { x += hpadding; }
                elements.at(index).position = {
                        affine::point2d{x, y}, ex.extents()};
                row_height = std::max(row_height, ex.height.value());
                x += ex.width.value();
                max_width = std::max(x, max_width);
                ++index;
            }
            float const width = max_width, height = row_height + y;
            /// TODO We could calculate better min/max here
            return constrained_type{width, height};
        }
    };


    template<typename C>
    breakable_row(C) -> breakable_row<C>;
    template<typename C>
    breakable_row(std::string_view, C) -> breakable_row<C>;
    template<typename C>
    breakable_row(C, float) -> breakable_row<C>;
    template<typename C>
    breakable_row(std::string_view, C, float) -> breakable_row<C>;
    template<typename C>
    breakable_row(C, float, float) -> breakable_row<C>;
    template<typename C>
    breakable_row(std::string_view, C, float, float) -> breakable_row<C>;


}
