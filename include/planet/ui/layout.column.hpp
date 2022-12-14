#pragma once


#include <planet/affine2d.hpp>
#include <planet/ui/helpers.hpp>


namespace planet::ui {


    /// A single wide column
    template<typename C>
    struct column {
        using collection_type = C;
        using box_type = typename collection_type::value_type;
        collection_type items;
        /// Padding between items in the column
        float padding = {};

        explicit column(collection_type c, float const p = {})
        : items{std::move(c)}, padding{p} {}

        affine::extents2d extents(affine::extents2d const outer) const {
            float max_width = {}, height = {};
            for (auto const &item : items) {
                auto const ex = item.extents(outer);
                max_width = std::max(max_width, ex.width);
                if (height) { height += padding; }
                height += ex.height;
            }
            return {max_width, height};
        }

        template<typename Target>
        auto draw_within(Target &t, affine::rectangle const outer) {
            auto const width = extents(outer.extents).width;
            float top = {};
            for (auto &item : items) {
                affine::extents2d const remaining = {
                        width, outer.bottom() - top};
                auto const ex = item.extents(remaining);
                item.draw_within(
                        t,
                        {outer.top_left + affine::point2d{0, top}, remaining});
                if (top) { top += padding; }
                top += ex.height;
            }
        }
    };


    template<typename... Pack>
    struct column<std::tuple<Pack...>> {
        using collection_type = std::tuple<Pack...>;
        collection_type items;
        float padding = {};

        column(collection_type i, float const p)
        : items{std::move(i)}, padding{p} {}

        affine::extents2d extents(affine::extents2d const outer) const {
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
        auto draw_within(Target &t, affine::rectangle const outer) {
            return draw_within(
                    t, outer, std::make_index_sequence<sizeof...(Pack)>{});
        }

      private:
        template<typename Target, std::size_t... I>
        void draw_within(
                Target &t,
                affine::rectangle const outer,
                std::index_sequence<I...>) {
            float top = outer.top();
            ((top += draw_item(t, std::get<I>(items), outer, top) + padding),
             ...);
        }
        template<typename Target, typename Item>
        float draw_item(
                Target &t,
                Item &item,
                affine::rectangle const within,
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
