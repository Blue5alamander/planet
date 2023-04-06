#pragma once


#include <planet/affine2d.hpp>
#include <planet/ui/helpers.hpp>
#include <planet/ui/layout.hpp>

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
    struct row<std::tuple<Pack...>> final {
        using collection_type = std::tuple<Pack...>;
        collection_type items;
        static constexpr std::size_t item_count =
                std::tuple_size<collection_type>();
        static_assert(
                item_count > 0,
                "There must be at least one UI element in the row");
        /// Padding between items in the row
        float padding = {};

        row(collection_type c, float const p)
        : items{std::move(c)}, padding{p} {}

        using layout_type = planet::ui::layout<
                std::array<planet::ui::element<void>, item_count>>;
        using constrained_type = typename layout_type::constrained_type;
        layout_type elements;

        auto reflow(constrained_type const &constraint) {
            auto const space = constraint.extents();
            float const unused = space.width - (item_count - 1) * padding;
            float const item_width = unused / item_count;
            float left = 0, max_height = {};
            auto const sizes = item_sizes(items, {item_width, space.height});
            for (std::size_t index{}; auto &element : elements) {
                element.position = {{left, {}}, sizes[index]};
                left += sizes[index].width + padding;
                max_height = std::max(max_height, sizes[index].height);
                ++index;
            }
            return affine::extents2d{left - padding, max_height};
        }

        affine::extents2d extents(affine::extents2d const outer) {
            return reflow(constrained_type{outer});
        }

        template<typename Target>
        void draw_within(Target &t, affine::rectangle2d const outer) {
            extents(outer.extents);
            return draw_within(
                    t, outer.top_left,
                    std::make_index_sequence<sizeof...(Pack)>{});
        }

      private:
        template<typename Target, std::size_t... I>
        void draw_within(
                Target &t,
                affine::point2d const offset,
                std::index_sequence<I...>) {
            (draw_item(t, std::get<I>(items), offset, I), ...);
        }
        template<typename Target, typename Item>
        void draw_item(
                Target &t,
                Item &item,
                affine::point2d const offset,
                std::size_t index) {
            item.draw_within(
                    t,
                    {offset + elements[index].position->top_left,
                     elements[index].position->extents});
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
