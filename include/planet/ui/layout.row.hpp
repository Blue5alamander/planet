#pragma once


#include <planet/affine2d.hpp>
#include <planet/ui/helpers.hpp>

#include <felspar/memory/small_vector.hpp>


namespace planet::ui {


    /// ## A row of boxes
    template<typename C>
    struct row {
        using collection_type = C;
        using box_type = typename collection_type::value_type;
        collection_type items;
        /// Padding between items in the row
        float padding = {};

        row(collection_type c, float const p)
        : items{std::move(c)}, padding{p} {}

        affine::extents2d extents(affine::extents2d const outer) {
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
        void draw_within(Target &t, affine::rectangle2d const outer) {
            auto left = outer.left();
            auto const top = outer.top(), bottom = outer.bottom();
            for (auto &item : items) {
                auto const ex = item.extents(outer.extents);
                auto const width = ex.width;
                item.draw_within(
                        t,
                        {{left, top}, affine::point2d{left + width, bottom}});
                left += width + padding;
            }
        }
    };
    template<typename... Pack>
    struct row<std::tuple<Pack...>> {
        using collection_type = std::tuple<Pack...>;
        collection_type items;
        static constexpr std::size_t item_count =
                std::tuple_size<collection_type>();
        /// Padding between items in the row
        float padding = {};

        row(collection_type c, float const p)
        : items{std::move(c)}, padding{p} {}

        affine::extents2d extents(affine::extents2d const outer) {
            if constexpr (item_count == 0) {
                return {{}, {}};
            } else {
                auto const sizes = item_sizes(items, outer);
                auto const first_ex = sizes[0];
                float width = first_ex.width;
                float height = first_ex.height;
                for (std::size_t index{1}; index < item_count; ++index) {
                    auto const ex = sizes[index];
                    width += padding + ex.width;
                    height = std::max(height, ex.height);
                }
                return {width, height};
            }
        }

        template<typename Target>
        void draw_within(Target &t, affine::rectangle2d const outer) {
            return draw_within(
                    t, outer, std::make_index_sequence<sizeof...(Pack)>{});
        }

      private:
        template<typename Target, std::size_t... I>
        void draw_within(
                Target &t,
                affine::rectangle2d const outer,
                std::index_sequence<I...>) {
            float left = outer.left();
            ((left += draw_item(t, std::get<I>(items), outer, left) + padding),
             ...);
        }
        template<typename Target, typename Item>
        float draw_item(
                Target &t,
                Item &item,
                affine::rectangle2d const within,
                float const left) {
            auto const ex = item.extents(within.extents);
            auto const height = ex.height;
            auto const top = within.top(), right = within.right();
            item.draw_within(
                    t, {{left, top}, affine::point2d{right, top + height}});
            return ex.width;
        }
    };


    /// ## Draws the items across multiple lines when needed
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
            felspar::memory::small_vector<affine::rectangle2d, sizeof...(Pack)>
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
