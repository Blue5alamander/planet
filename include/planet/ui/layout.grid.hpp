#pragma once


#include <planet/affine2d.hpp>
#include <planet/ui/helpers.hpp>
#include <planet/ui/reflowable.hpp>

#include <felspar/memory/small_vector.hpp>


namespace planet::ui {


    /// A grid of equally sized blocks which can break onto multiple lines
    template<typename C>
    struct grid {
        using collection_type = C;
        collection_type items;
        /// Padding between items in the row
        float vpadding = {}, hpadding = {};

        grid(collection_type c, float const p)
        : items{std::move(c)}, vpadding{p}, hpadding{p} {}

        affine::extents2d extents(affine::extents2d const outer) {
            auto const cell = cell_size(outer);
            auto const w = std::min(
                    float(items.size()),
                    std::floor(
                            (outer.width + hpadding) / (cell.width + hpadding)));
            auto const h = std::floor((items.size() + w - 1) / w);
            return {w * cell.width + (w - 1) * hpadding,
                    h * cell.height + (h - 1) * vpadding};
        }

        template<typename Target>
        void draw_within(Target &t, affine::rectangle2d const within) {
            auto const cell = cell_size(within.extents);
            float x = {}, y = {};
            for (auto &item : items) {
                if (x > 0 and x + cell.width > within.extents.width) {
                    x = 0;
                    if (y) { y += vpadding; }
                    y += cell.height;
                }
                item.draw_within(
                        t, {affine::point2d{x, y} + within.top_left, cell});
                if (x) { x += hpadding; }
                x += cell.width;
            }
        }

      private:
        affine::extents2d cell_size(affine::extents2d const outer) const {
            float max_width = {}, max_height = {};
            for (auto &i : items) {
                auto const ex = i.extents(outer);
                max_width = std::max(ex.width, max_width);
                max_height = std::max(ex.height, max_height);
            }
            return {max_width, max_height};
        }
    };
    template<typename... Pack>
    struct grid<std::tuple<Pack...>> : public reflowable {
        using collection_type = std::tuple<Pack...>;
        collection_type items;
        /// Padding between items in the row
        float vpadding = {}, hpadding = {};

        grid(collection_type c, float const p)
        : items{std::move(c)}, vpadding{p}, hpadding{p} {}

        affine::extents2d extents(affine::extents2d const outer) {
            auto const cell = cell_size(outer);
            auto const w = std::min(
                    float(sizeof...(Pack)),
                    std::floor(
                            (outer.width + hpadding) / (cell.width + hpadding)));
            auto const h = std::floor((sizeof...(Pack) + w - 1) / w);
            return {w * cell.width + (w - 1) * hpadding,
                    h * cell.height + (h - 1) * vpadding};
        }

        template<typename Target>
        void draw_within(Target &t, affine::rectangle2d const within) {
            auto const cell = cell_size(within.extents);
            float x = {}, y = {};
            felspar::memory::small_vector<affine::rectangle2d, sizeof...(Pack)>
                    locations;
            for (std::size_t index{}; index < sizeof...(Pack); ++index) {
                if (x > 0 and x + cell.width > within.extents.width) {
                    x = 0;
                    if (y) { y += vpadding; }
                    y += cell.height;
                }
                locations.emplace_back(
                        affine::point2d{x, y} + within.top_left, cell);
                if (x) { x += hpadding; }
                x += cell.width;
            }
            draw_items_within(
                    t, items, locations,
                    std::make_index_sequence<sizeof...(Pack)>{});
        }

      private:
        constrained_type do_reflow(constrained_type const &ex) {
            /// TODO All of the layout logic should move to here which will fill
            /// in a `layout` structure
            return constrained_type{extents(ex.extents())};
        }

        affine::extents2d cell_size(affine::extents2d const outer) {
            float max_width = {}, max_height = {};
            for (auto &ex : item_sizes(items, outer)) {
                max_width = std::max(ex.width, max_width);
                max_height = std::max(ex.height, max_height);
            }
            return {max_width, max_height};
        }
    };


}
