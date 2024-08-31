#pragma once


#include <planet/ui/pack.reflowable.hpp>


namespace planet::ui {


    /// ## A grid of equally sized blocks which can break onto multiple lines
    /**
     * The size of the grid squares is based on the minimum width and height of
     * the contained grid items.
     */
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
    struct grid<std::tuple<Pack...>> final :
    public pack_reflowable<void, Pack...> {
        using superclass = pack_reflowable<void, Pack...>;
        using collection_type = typename superclass::collection_type;
        using constrained_type = typename superclass::constrained_type;
        using reflow_parameters = typename superclass::reflow_parameters;
        using superclass::elements;
        using superclass::item_count;
        using superclass::items;


        grid(collection_type c, float const p)
        : superclass{"planet::ui::grid<std::tuple<Pack...>>", std::move(c)},
          vpadding{p},
          hpadding{p} {}


        /// ### Padding between items
        float vpadding = {}, hpadding = {};


      private:
        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &border) override {
            auto const constraints = superclass::items_reflow(p, border);
            affine::extents2d min;
            for (auto const &ic : constraints) {
                min.width = std::max(min.width, ic.width.min());
                min.height = std::max(min.height, ic.height.min());
            }
            float left = 0, top = 0, max_width = 0;
            for (auto &element : elements) {
                if (left + hpadding + min.width > border.width.max()) {
                    max_width = std::max(left, max_width);
                    left = 0;
                    top += min.height + hpadding;
                }
                if (left) { left += hpadding; }
                element.position = {{left, top}, min};
                left += min.width;
            }
            auto const width = std::max(max_width, left);
            auto const height = top + min.height;
            return constrained_type{
                    {width, width, width}, {height, height, height}};
        }
    };


}
