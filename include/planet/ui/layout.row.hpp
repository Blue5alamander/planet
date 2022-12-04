#pragma once


#include <planet/affine2d.hpp>
#include <planet/ui/helpers.hpp>

#include <felspar/memory/small_vector.hpp>


namespace planet::ui {


    /// A row of boxes. When presented they can line break pushing over spill
    /// into the space below
    template<typename C>
    struct row {
        using collection_type = C;
        using box_type = typename collection_type::value_type;
        collection_type items;
        /// Padding between items in the row
        float padding = {};

        row(collection_type c, float const p)
        : items{std::move(c)}, padding{p} {}

        affine::extents2d extents(affine::extents2d const outer) const {
            auto const first_ex = items[0].extents(outer);
            float width = first_ex.width;
            float height = first_ex.height;
            for (std::size_t index{1}; index < items.size(); ++index) {
                auto const ex = items[index].extents(outer);
                width += padding + ex.width;
                height = std::max(height, ex.height);
            }
            return {width, height};
        }

        template<typename Target>
        void draw_within(Target &t, affine::rectangle const outer) {
            auto left = outer.left();
            auto const top = outer.top(), bottom = outer.bottom();
            for (auto const &item : items) {
                auto const ex = item.extents(outer.extents);
                auto const width = ex.width;
                item.draw_within(
                        t,
                        {{left, top}, affine::point2d{left + width, bottom}});
                left += width + padding;
            }
        }
    };


    /// Draws the items across multiple lines when needed
    template<typename C>
    struct breakable_row {
        using collection_type = C;
        collection_type items;
        /// Padding between items in the row
        float hpadding = {}, vpadding = {};

        breakable_row(collection_type c, float const hp, float const vp)
        : items{std::move(c)}, hpadding{hp}, vpadding{vp} {}
        breakable_row(collection_type c, float const p)
        : breakable_row{std::move(c), p, p} {}

        affine::extents2d extents(affine::extents2d const outer) const {
            float max_width = {}, row_height = {}, total_height = {}, left{};
            for (auto const &item : items) {
                auto const item_ex = item.extents(outer);
                if (left + item_ex.width > outer.width) {
                    max_width = std::max(max_width, left);
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
        void draw_within(Target &t, affine::rectangle const border) {
            float row_height = {}, x = {}, y = {};
            for (auto &item : items) {
                auto const ex = item.extents(border.extents);
                if (x + ex.width > border.extents.width) {
                    x = {};
                    if (y) { y += vpadding; }
                    y += row_height;
                    row_height = {};
                }
                if (x) { x += hpadding; }
                planet::affine::rectangle const location = {
                        border.top_left + affine::point2d{x, y}, ex};
                item.draw_within(t, location);
                row_height = std::max(row_height, ex.height);
                x += ex.width;
            }
        }
    };
    template<typename... Pack>
    struct breakable_row<std::tuple<Pack...>> {
        using collection_type = std::tuple<Pack...>;
        collection_type items;
        /// Padding between items in the row
        float hpadding = {}, vpadding = {};

        breakable_row(collection_type c, float const hp, float const vp)
        : items{std::move(c)}, hpadding{hp}, vpadding{vp} {}
        breakable_row(collection_type c, float const p)
        : breakable_row{std::move(c), p, p} {}

        affine::extents2d extents(affine::extents2d const outer) const {
            float max_width = {}, row_height = {}, total_height = {}, left{};
            for (auto const &item_ex : item_sizes(items, outer)) {
                if (left + item_ex.width > outer.width) {
                    max_width = std::max(max_width, left);
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
        void draw_within(Target &t, affine::rectangle const border) {
            float row_height = {}, x = {}, y = {};
            felspar::memory::small_vector<affine::rectangle, sizeof...(Pack)>
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
                    std::make_index_sequence<sizeof...(Pack)>{});
        }
    };


}
