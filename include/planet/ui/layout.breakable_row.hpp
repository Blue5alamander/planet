#pragma once


#include <planet/ui/collection.reflowable.hpp>
#include <planet/ui/pack.reflowable.hpp>


namespace planet::ui {


    /// ## Draws the items across multiple lines when needed
    template<typename CT>
    struct breakable_row : public collection_reflowable<CT, void> {
        using superclass = collection_reflowable<CT, void>;
        using collection_type = typename superclass::collection_type;
        using constrained_type = typename superclass::constrained_type;
        using superclass::elements;
        using superclass::items;

        /// Padding between items in the row
        float hpadding = {}, vpadding = {};

        explicit breakable_row(collection_type c, float const hp, float const vp)
        : superclass{"planet::ui::breakable_row<C>", std::move(c)},
          hpadding{hp},
          vpadding{vp} {}
        explicit breakable_row(
                std::string_view const n,
                collection_type c,
                float const hp,
                float const vp)
        : superclass{n, std::move(c)}, hpadding{hp}, vpadding{vp} {}
        explicit breakable_row(collection_type c, float const p)
        : breakable_row{std::move(c), p, p} {}
        explicit breakable_row(
                std::string_view const n, collection_type c, float const p)
        : breakable_row{n, std::move(c), p, p} {}

        affine::extents2d extents(affine::extents2d const outer) {
            float max_width = {}, row_height = {}, total_height = {}, left{};
            for (auto const &item : items) {
                auto const item_ex = item.extents(outer);
                if (left and left + item_ex.width > outer.width) {
                    max_width = std::max(max_width, left - hpadding);
                    if (total_height) { total_height += vpadding; }
                    total_height += row_height;
                    left = {};
                    row_height = {};
                } else {
                    if (left) { left += hpadding; }
                    left += item_ex.width;
                }
            }
            max_width = std::max(max_width, left);
            total_height += row_height;
            return {max_width, total_height};
        }

        template<typename Target>
        void draw_within(Target &t, affine::rectangle2d const border) {
            float row_height = {}, x = {}, y = {};
            for (auto &item : items) {
                auto const ex = item.extents(border.extents);
                if (x and x + ex.width > border.extents.width) {
                    x = {};
                    if (y) { y += vpadding; }
                    y += row_height;
                    row_height = {};
                }
                planet::affine::rectangle2d const location = {
                        border.top_left + affine::point2d{x, y}, ex};
                item.draw_within(t, location);
                row_height = std::max(row_height, ex.height);
                x += ex.width;
                if (x) { x += hpadding; }
            }
        }

      private:
        constrained_type do_reflow(constrained_type const &border) override {
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
    struct breakable_row<std::tuple<Pack...>> :
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

        affine::extents2d extents(affine::extents2d const outer) {
            float max_width = {}, row_height = {}, total_height = {}, left{};
            for (auto &item_ex : item_sizes(items, outer)) {
                if (left + item_ex.width > outer.width) {
                    max_width = std::max(max_width, left);
                    if (total_height) { total_height += vpadding; }
                    total_height += row_height;
                    left = {};
                    row_height = {};
                } else {
                    if (left) { left += hpadding; }
                    left += item_ex.width;
                    row_height = std::max(row_height, item_ex.height);
                }
            }
            max_width = std::max(max_width, left);
            total_height += row_height;
            return {max_width, total_height};
        }

        template<typename Target>
        void draw_within(Target &t, affine::rectangle2d const border) {
            float row_height = {}, x = {}, y = {};
            felspar::memory::small_vector<affine::rectangle2d, item_count>
                    locations;
            for (auto &ex : item_sizes(items, border.extents)) {
                if (x + ex.width > border.extents.width) {
                    x = {};
                    if (y) { y += vpadding; }
                    y += row_height;
                    row_height = {};
                }
                if (x) { x += hpadding; }
                locations.emplace_back(
                        border.top_left + affine::point2d{x, y}, ex);
                row_height = std::max(row_height, ex.height);
                x += ex.width;
            }
            draw_items_within(
                    t, items, locations,
                    std::make_index_sequence<item_count>{});
        }

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
