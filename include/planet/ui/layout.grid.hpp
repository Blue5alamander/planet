#pragma once


#include <planet/ui/collection.reflowable.hpp>
#include <planet/ui/pack.reflowable.hpp>


namespace planet::ui {


    /// ## A grid of equally sized blocks which can break onto multiple lines
    /**
     * The size of the grid squares is based on the minimum width and height of
     * the contained grid items.
     */
    template<typename C>
    struct grid final : collection_reflowable<C, void> {
        using superclass = collection_reflowable<C, void>;
        using collection_type = typename superclass::collection_type;
        using constrained_type = typename superclass::constrained_type;
        using reflow_parameters = typename superclass::reflow_parameters;


        grid(collection_type c, float const p = {})
        : superclass{"planet::ui::grid<>", std::move(c)},
          vpadding{p},
          hpadding{p} {}
        grid(collection_type c, float const h, float const v)
        : superclass{"planet::ui::grid<>", std::move(c)},
          vpadding{v},
          hpadding{h} {}
        grid(collection_type c, affine::extents2d const ex)
        : superclass{"planet::ui::grid<>", std::move(c)},
          vpadding{ex.height},
          hpadding{ex.width} {}


        using superclass::elements;
        using superclass::items;
        /// Padding between items in the row
        float vpadding = {}, hpadding = {};


      private:
        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &constraint) override {
            auto const cell = cell_size(p, constraint);
            auto const w = std::min(
                    float(items.size()),
                    std::floor(
                            (constraint.width.value() + hpadding)
                            / (cell.width + hpadding)));
            auto const h = std::floor((items.size() + w - 1) / w);
            constrained_type size{
                    w * (cell.width + hpadding) - hpadding,
                    h * (cell.height + vpadding) - vpadding};
            arrange_elements(cell, size);
            return size;
        }
        void arrange_elements(
                affine::extents2d const cell, constrained_type const within) {
            float x = {}, y = {};
            for (auto &element : elements) {
                if (x > 0 and x + cell.width > within.extents().width) {
                    x = 0;
                    y += cell.height + vpadding;
                }
                element.position = {affine::point2d{x, y}, cell};
                x += cell.width + hpadding;
            }
        }

        affine::extents2d cell_size(
                reflow_parameters const &p,
                constrained_type const &constraint) {
            float max_width = {}, max_height = {};
            elements.resize_to(items);
            for (std::size_t index{}; auto &i : items) {
                auto const ex = i.reflow(p, constraint);
                elements.at(index).constraints = ex;
                max_width = std::max(ex.width.value(), max_width);
                max_height = std::max(ex.height.value(), max_height);
                ++index;
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


        grid(collection_type c, float const p = {})
        : superclass{"planet::ui::grid<std::tuple<Pack...>>", std::move(c)},
          vpadding{p},
          hpadding{p} {}
        grid(collection_type c, float const h, float const v)
        : superclass{"planet::ui::grid<std::tuple<Pack...>>", std::move(c)},
          vpadding{v},
          hpadding{h} {}


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


    template<typename C>
    grid(C) -> grid<C>;
    template<typename C>
    grid(C, float) -> grid<C>;
    template<typename C>
    grid(C, affine::extents2d) -> grid<C>;
    template<typename C>
    grid(C, float, float) -> grid<C>;


}
