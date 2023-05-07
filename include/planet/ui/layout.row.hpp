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
        using superclass::elements;
        using superclass::items;

        /// Padding between items in the row
        float padding = {};

        explicit row(collection_type c, float const p)
        : superclass{"planet::ui::row<>", std::move(c)}, padding{p} {}
        explicit row(std::string_view const n, collection_type c, float const p)
        : superclass{n, std::move(c)}, padding{p} {}

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

      private:
        constrained_type do_reflow(constrained_type const &bounds) override {
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
                auto const ex = item.reflow(space);
                elements.at(index).position = {{left, 0}, ex.extents()};
                left += ex.width.value() + padding;
                max_height = std::max(max_height, ex.height.value());
                ++index;
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
        using superclass::elements;
        using superclass::item_count;
        using superclass::items;

        /// Padding between items in the row
        float padding = {};

        explicit row(collection_type c, float const p)
        : superclass{"planet::ui::row<std::tuple<Pack...>>", std::move(c)},
          padding{p} {}
        explicit row(std::string_view const n, collection_type c, float const p)
        : superclass{n, std::move(c)}, padding{p} {}

        affine::extents2d extents(affine::extents2d const outer) {
            return reflow(constrained_type{outer}).extents();
        }

        template<typename Target>
        void draw_within(Target &t, affine::rectangle2d const outer) {
            extents(outer.extents);
            return draw_within(
                    t, outer.top_left,
                    std::make_index_sequence<sizeof...(Pack)>{});
        }

      private:
        constrained_type do_reflow(constrained_type const &constraint) override {
            /// TODO Use constrained type when calculating the `item_sizes` and
            /// also when returning the constraint value
            auto const space = constraint.extents();
            float const unused = space.width - (item_count - 1) * padding;
            float const item_width = unused / item_count;
            float left = 0, max_height = {};
            auto const sizes = item_sizes(
                    items, affine::extents2d{item_width, space.height});
            for (std::size_t index{}; auto &element : elements) {
                element.position = {{left, {}}, sizes[index]};
                left += sizes[index].width + padding;
                max_height = std::max(max_height, sizes[index].height);
                ++index;
            }
            /// TODO Calculate better constraints here
            return constrained_type{
                    affine::extents2d{left - padding, max_height}};
        }

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
                    {offset + elements.at(index).position->top_left,
                     elements.at(index).position->extents});
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
