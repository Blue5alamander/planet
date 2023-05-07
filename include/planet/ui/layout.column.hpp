#pragma once


#include <planet/ui/pack.reflowable.hpp>


namespace planet::ui {


    /// ## A single wide column
    template<typename C>
    struct column : public reflowable {
        using collection_type = C;
        using box_type = typename collection_type::value_type;
        collection_type items;
        /// ### Padding between items in the column
        float padding = {};

        column() {}
        explicit column(collection_type c, float const p = {})
        : reflowable{"planet::ui::column<C>"}, items{std::move(c)}, padding{p} {}
        explicit column(
                std::string_view const n, collection_type c, float const p = {})
        : reflowable{n}, items{std::move(c)}, padding{p} {}

        using layout_type = planet::ui::layout_for<C>;
        using constrained_type = typename layout_type::constrained_type;
        layout_type elements;

        affine::extents2d extents(affine::extents2d const outer) {
            return reflow(constrained_type{outer}).extents();
        }

        template<typename Target>
        auto draw_within(Target &t, affine::rectangle2d const outer) {
            /// TODO Should use dirty handling
            reflow(constrained_type{outer.extents});
            for (std::size_t index{}; auto &item : items) {
                if (index < elements.size() and elements.at(index).position) {
                    auto const &pos = *elements.at(index).position;
                    item.draw_within(
                            t, {outer.top_left + pos.top_left, pos.extents});
                }
                ++index;
            }
        }

      private:
        constrained_type do_reflow(constrained_type const &constraint) override {
            /// TODO Use constrained type when calculating the `item_sizes` and
            /// also when returning the constraint value
            if (items.empty()) {
                elements.extents = affine::extents2d{{}, {}};
                return {};
            } else {
                auto const space = constraint.extents();
                float const unused =
                        space.height - (items.size() - 1) * padding;
                float const item_height = unused / items.size();
                float top = {}, max_width = {};
                affine::extents2d const row_space{space.width, item_height};
                elements.resize_to(std::span{items});
                for (std::size_t index{}; auto &item : items) {
                    affine::extents2d const item_ex =
                            ui::reflow(item, row_space);
                    elements.at(index).size = constrained_type{item_ex};
                    elements.at(index).position = {{{}, top}, item_ex};
                    top += item_ex.height + padding;
                    max_width = std::max(max_width, item_ex.width);
                    ++index;
                }
                elements.extents = {max_width, top - padding};
                return constrained_type{*elements.extents};
            }
        }

        void move_sub_elements(affine::rectangle2d const &) override {}
    };


    /// ## Specialisation for tuple
    template<typename... Pack>
    struct column<std::tuple<Pack...>> : public pack_reflowable<void, Pack...> {
        using superclass = pack_reflowable<void, Pack...>;
        using collection_type = typename superclass::collection_type;
        using constrained_type = typename superclass::constrained_type;
        using superclass::elements;
        using superclass::item_count;
        using superclass::items;

        /// Padding between items
        float padding = {};

        explicit column(
                collection_type i,
                float const p,
                felspar::source_location const &loc =
                        felspar::source_location::current())
        : superclass{"planet::ui::column<std::tuple<Pack...>>", std::move(i), loc},
          padding{p} {}
        explicit column(
                std::string_view const n,
                collection_type i,
                float const p,
                felspar::source_location const &loc =
                        felspar::source_location::current())
        : superclass{n, std::move(i), loc}, padding{p} {}

        affine::extents2d extents(affine::extents2d const outer) {
            auto const sizes = item_sizes(items, outer);
            affine::extents2d r{{}, {}};
            for (auto const e : sizes) {
                r.width = std::max(r.width, e.width);
                if (r.height) { r.height += padding; }
                r.height += e.height;
            }
            return r;
        }

        template<typename Target>
        auto draw_within(Target &t, affine::rectangle2d const outer) {
            return draw_within(
                    t, outer, std::make_index_sequence<sizeof...(Pack)>{});
        }

      private:
        constrained_type do_reflow(constrained_type const &ex) override {
            float const unused = ex.height.value() - (item_count - 1) * padding;
            float const item_height = unused / item_count;
            float top = {}, max_width = {};
            auto const sizes = superclass::items_reflow(
                    {ex.width, {ex.height.min(), item_height, ex.height.max()}});
            for (std::size_t index{}; auto &element : elements) {
                element.position = {{{}, top}, sizes[index].extents()};
                top += sizes[index].height.value() + padding;
                max_width = std::max(max_width, sizes[index].width.value());
                ++index;
            }
            return constrained_type{max_width, top - padding};
        }

        template<typename Target, std::size_t... I>
        void draw_within(
                Target &t,
                affine::rectangle2d const outer,
                std::index_sequence<I...>) {
            float top = outer.top();
            ((top += draw_item(t, std::get<I>(items), outer, top) + padding),
             ...);
        }
        template<typename Target, typename Item>
        float draw_item(
                Target &t,
                Item &item,
                affine::rectangle2d const within,
                float const top) {
            auto const ex = item.extents(within.extents);
            auto const height = ex.height;
            auto const left = within.left(), right = within.right();
            item.draw_within(
                    t, {{left, top}, affine::point2d{right, top + height}});
            return height;
        }
    };


}
