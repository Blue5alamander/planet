#pragma once


#include <planet/ui/collection.reflowable.hpp>
#include <planet/ui/pack.reflowable.hpp>


namespace planet::ui {


    /// ## A single wide column
    template<typename CT>
    struct column : public collection_reflowable<CT, void> {
        using superclass = collection_reflowable<CT, void>;
        using collection_type = typename superclass::collection_type;
        using constrained_type = typename superclass::constrained_type;
        using superclass::elements;
        using superclass::items;

        /// ### Padding between items in the column
        float padding = {};

        column() {}
        explicit column(collection_type c, float const p = {})
        : superclass{"planet::ui::column<>", std::move(c)}, padding{p} {}
        explicit column(
                std::string_view const n, collection_type c, float const p = {})
        : superclass{n, std::move(c)}, padding{p} {}

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
    };


    template<typename C>
    column(C) -> column<C>;
    template<typename C>
    column(std::string_view, C) -> column<C>;
    template<typename C>
    column(C, float) -> column<C>;
    template<typename C>
    column(std::string_view, C, float) -> column<C>;


}
