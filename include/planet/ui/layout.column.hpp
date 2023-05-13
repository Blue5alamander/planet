#pragma once


#include <planet/ui/collection.reflowable.hpp>
#include <planet/ui/pack.reflowable.hpp>


namespace planet::ui {


    /// ## A single wide column
    template<typename CT>
    struct column final : public collection_reflowable<CT, void> {
        using superclass = collection_reflowable<CT, void>;
        using collection_type = typename superclass::collection_type;
        using constrained_type = typename superclass::constrained_type;
        using superclass::elements;
        using superclass::items;

        /// ### Padding between items in the column
        float padding = {};

        column() : superclass{"planet::ui::column<>"} {}
        explicit column(collection_type c, float const p = {})
        : superclass{"planet::ui::column<>", std::move(c)}, padding{p} {}
        explicit column(
                std::string_view const n, collection_type c, float const p = {})
        : superclass{n, std::move(c)}, padding{p} {}

      private:
        constrained_type do_reflow(constrained_type const &constraint) override {
            if (items.empty()) {
                elements.extents = affine::extents2d{{}, {}};
                return {};
            } else {
                auto const space = constraint.extents();
                float const unused =
                        space.height - (items.size() - 1) * padding;
                float const item_height = unused / items.size();
                float top = {}, max_width = {};
                constrained_type const row_space = {
                        constraint.width,
                        {constraint.height.min(), item_height,
                         constraint.height.max()}};
                elements.resize_to(std::span{items});
                for (std::size_t index{}; auto &item : items) {
                    auto const item_ex = item.reflow(row_space);
                    elements.at(index).size = item_ex;
                    elements.at(index).position = {
                            {{}, top}, item_ex.extents()};
                    top += item_ex.height.value() + padding;
                    max_width = std::max(max_width, item_ex.width.value());
                    ++index;
                }
                elements.extents = {max_width, top - padding};
                return constrained_type{*elements.extents};
            }
        }
    };


    /// ## Specialisation for tuple
    template<typename... Pack>
    struct column<std::tuple<Pack...>> final :
    public pack_reflowable<void, Pack...> {
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
                float const p = {},
                felspar::source_location const &loc =
                        felspar::source_location::current())
        : superclass{"planet::ui::column<std::tuple<Pack...>>", std::move(i), loc},
          padding{p} {}
        explicit column(
                std::string_view const n,
                collection_type i,
                float const p = {},
                felspar::source_location const &loc =
                        felspar::source_location::current())
        : superclass{n, std::move(i), loc}, padding{p} {}

      private:
        constrained_type do_reflow(constrained_type const &ex) override {
            float const unused = ex.height.value() - (item_count - 1) * padding;
            float const item_height = unused / item_count;
            float top = {}, max_width = {};
            constrained_type const row_space = {
                    ex.width, {ex.height.min(), item_height, ex.height.max()}};
            auto const sizes = superclass::items_reflow(row_space);
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
