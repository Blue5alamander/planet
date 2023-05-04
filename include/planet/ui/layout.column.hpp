#pragma once


#include <planet/affine2d.hpp>
#include <planet/ui/helpers.hpp>
#include <planet/ui/layout.hpp>
#include <planet/ui/reflowable.hpp>


namespace planet::ui {


    /// ## A single wide column
    template<typename C>
    struct column {
        using collection_type = C;
        using box_type = typename collection_type::value_type;
        collection_type items;
        /// ### Padding between items in the column
        float padding = {};

        column() {}
        explicit column(collection_type c, float const p = {})
        : items{std::move(c)}, padding{p} {}

        using layout_type = planet::ui::layout_for<C>;
        using constrained_type = typename layout_type::constrained_type;
        layout_type elements;

        constrained_type reflow(constrained_type const &constraint) {
            /// TODO Use constrained type when calculating the `item_sizes` and
            /// also when returning the constriant value
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
    };


    /// ## Specialisation for tuple
    template<typename... Pack>
    struct column<std::tuple<Pack...>> : public reflowable {
        using collection_type = std::tuple<Pack...>;
        collection_type items;
        float padding = {};

        column(collection_type i, float const p)
        : items{std::move(i)}, padding{p} {}

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
        constrained_type do_reflow(constrained_type const &ex) {
            /// TODO All of the layout logic should move to here which will fill
            /// in a `layout` structure
            return constrained_type{extents(ex.extents())};
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
