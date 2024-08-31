#pragma once


#include <planet/ui/collection.reflowable.hpp>
#include <planet/ui/pack.reflowable.hpp>


namespace planet::ui {


    /// ## A row of boxes
    template<typename CT>
    struct row final : public collection_reflowable<CT, void> {
        using superclass = collection_reflowable<CT, void>;
        using collection_type = typename superclass::collection_type;
        using constrained_type = typename superclass::constrained_type;
        using reflow_parameters = typename superclass::reflow_parameters;
        using superclass::elements;
        using superclass::items;


        /// Padding between items in the row
        float padding = {};


        row(collection_type c, float const p = {})
        : superclass{"planet::ui::row<>", std::move(c)}, padding{p} {}
        row(std::string_view const n, collection_type c, float const p = {})
        : superclass{n, std::move(c)}, padding{p} {}


      private:
        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &bounds) override {
            elements.resize_to(std::span{items});
            if (items.empty()) { return {}; }
            float const unused =
                    bounds.width.value() - (items.size() - 1) * padding;
            float const item_width = unused / items.size();
            constrained_type const space{
                    {item_width, std::min(item_width, bounds.width.min()),
                     bounds.width.max()},
                    bounds.height};
            float left = 0, max_height = 0;
            for (std::size_t index{}; auto &item : items) {
                auto const ex = item.reflow(p, space);
                elements.at(index).constraints = ex;
                max_height = std::max(max_height, ex.height.value());
                ++index;
            }
            for (auto &element : elements) {
                element.position = {
                        {left, 0},
                        affine::extents2d{
                                element.constraints.extents().width,
                                max_height}};
                left += element.constraints.width.value() + padding;
            }
            return constrained_type{left - padding, max_height};
        }
    };


    template<typename... Pack>
    struct row<std::tuple<Pack...>> final :
    public pack_reflowable<void, Pack...> {
        using superclass = pack_reflowable<void, Pack...>;
        using collection_type = typename superclass::collection_type;
        using constrained_type = typename superclass::constrained_type;
        using reflow_parameters = typename superclass::reflow_parameters;
        using superclass::elements;
        using superclass::item_count;
        using superclass::items;


        /// ### Padding between items in the row
        float padding = {};


        row(collection_type c, float const p = {})
        : superclass{"planet::ui::row<std::tuple<Pack...>>", std::move(c)},
          padding{p} {}
        row(std::string_view const n, collection_type c, float const p = {})
        : superclass{n, std::move(c)}, padding{p} {}


      private:
        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &constraint) override {
            float const unused =
                    constraint.width.value() - (item_count - 1) * padding;
            float const item_width = unused / item_count;
            constrained_type const space{
                    {item_width, std::min(item_width, constraint.width.min()),
                     constraint.width.max()},
                    constraint.height};
            float left = 0, max_height = {};
            auto const sizes = superclass::items_reflow(p, space);
            for (auto &item : sizes) {
                max_height = std::max(max_height, item.height.value());
            }
            for (std::size_t index{}; auto &element : elements) {
                element.position = {
                        {left, {}},
                        affine::extents2d{
                                sizes[index].extents().width, max_height}};
                left += sizes[index].width.value() + padding;
                ++index;
            }
            return constrained_type{
                    affine::extents2d{left - padding, max_height}};
        }
    };


    template<typename C>
    row(C) -> row<C>;
    template<typename C>
    row(std::string_view, C) -> row<C>;
    template<typename C>
    row(C, float) -> row<C>;
    template<typename C>
    row(std::string_view, C, float) -> row<C>;


}
