#pragma once


#include <planet/ui/pack.reflowable.hpp>


namespace planet::ui {


    /// ## A row of boxes
    template<typename C>
    struct row final : public reflowable {
        using collection_type = C;
        using box_type = typename collection_type::value_type;
        collection_type items;

        /// Padding between items in the row
        float padding = {};

        explicit row(collection_type c, float const p)
        : reflowable{"planet::ui::row<C>"}, items{std::move(c)}, padding{p} {}
        explicit row(std::string_view const n, collection_type c, float const p)
        : reflowable{n}, items{std::move(c)}, padding{p} {}

        using layout_type = planet::ui::layout_for<C>;
        using constrained_type = typename layout_type::constrained_type;
        layout_type elements;

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

        void move_sub_elements(affine::rectangle2d const &r) override {
            for (std::size_t index{}; auto &item : items) {
                auto const &epos = elements.at(index).position.value();
                item.move_to({r.top_left + epos.top_left, epos.extents});
                ++index;
            }
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


    /// ## Draws the items across multiple lines when needed
    template<typename C>
    struct breakable_row : public reflowable {
        using collection_type = C;
        collection_type items;
        /// Padding between items in the row
        float hpadding = {}, vpadding = {};

        explicit breakable_row(collection_type c, float const hp, float const vp)
        : reflowable{"planet::ui::breakable_row<C>"},
          items{std::move(c)},
          hpadding{hp},
          vpadding{vp} {}
        explicit breakable_row(
                std::string_view const n,
                collection_type c,
                float const hp,
                float const vp)
        : reflowable{n}, items{std::move(c)}, hpadding{hp}, vpadding{vp} {}
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

        template<typename Renderer>
        void draw(Renderer &r) {
            for (auto &item : items) { item.draw(r); }
        }

      private:
        constrained_type do_reflow(constrained_type const &ex) override {
            /// TODO All of the layout logic should move to here which will fill
            /// in a `layout` structure
            return constrained_type{extents(ex.extents())};
        }

        void move_sub_elements(affine::rectangle2d const &) override {}
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


}
