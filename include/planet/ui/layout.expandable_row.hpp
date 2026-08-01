#pragma once


#include <planet/ui/pack.reflowable.hpp>

#include <felspar/exceptions/logic_error.hpp>


namespace planet::ui {


    /// ## A row where one item takes the space the others leave
    /**
     * Lays out like `planet::ui::row`, except that the item at `expanding` is
     * offered all of the width that the other items leave. The offer is made
     * through `move_to`, and the rectangle that `move_to` returns is what the
     * items to its right are laid out against, so the offer may also be
     * declined -- an item that keeps its own size leaves the row exactly as
     * narrow as a `row` would be. The row is never wider than the space it is
     * given, but it doesn't claim space that it hasn't managed to give away.
     */
    template<typename CT>
    struct expandable_row;


    template<typename... Pack>
    struct expandable_row<std::tuple<Pack...>> final :
    public pack_reflowable<void, Pack...> {
        using superclass = pack_reflowable<void, Pack...>;
        using collection_type = typename superclass::collection_type;
        using constrained_type = typename superclass::constrained_type;
        using reflow_parameters = typename superclass::reflow_parameters;
        using superclass::elements;
        using superclass::item_count;
        using superclass::item_sequence;
        using superclass::items;
        using superclass::name;


        /// ### Construction
        expandable_row(
                std::size_t const e,
                collection_type c,
                float const p = {},
                std::source_location const &loc =
                        std::source_location::current())
        : superclass{"planet::ui::expandable_row<std::tuple<Pack...>>", std::move(c), loc},
          expanding{e},
          padding{p} {}
        expandable_row(
                std::string_view const n,
                std::size_t const e,
                collection_type c,
                float const p = {},
                std::source_location const &loc =
                        std::source_location::current())
        : superclass{n, std::move(c), loc}, expanding{e}, padding{p} {}


        /// ### The item that expands
        std::size_t expanding;
        /**
         * The index of the item that is offered all of the width that the
         * other items leave. It must be one of the items in the row.
         */

        /// ### Padding between items in the row
        float padding = {};


      private:
        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &bounds) override {
            check_expanding();
            float const gaps = (item_count - 1) * padding;
            float const share = (bounds.width.value() - gaps) / item_count;
            constrained_type const space{
                    {share, std::min(share, bounds.width.min()),
                     bounds.width.max()},
                    bounds.height};
            float const used = reflow_others(p, space, item_sequence);
            float const offer =
                    std::max(bounds.width.value() - gaps - used, 0.0f);
            reflow_expanding(
                    p,
                    constrained_type{
                            {offer, std::min(offer, bounds.width.min()), offer},
                            bounds.height},
                    item_sequence);

            auto const sizes = superclass::items_constraints();
            float max_height = {};
            for (auto const &ex : sizes) {
                max_height = std::max(max_height, ex.height.value());
            }
            float left = {};
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

        affine::rectangle2d move_sub_elements(
                reflow_parameters const &p,
                affine::rectangle2d const &r) override {
            check_expanding();
            auto const sizes = superclass::items_constraints();
            float max_height = {}, used = (item_count - 1) * padding;
            for (std::size_t index{}; auto const &ex : sizes) {
                max_height = std::max(max_height, ex.height.value());
                if (index != expanding) { used += ex.width.value(); }
                ++index;
            }
            float const offer = std::max(r.extents.width - used, 0.0f);
            float const width =
                    move_items(p, r, sizes, offer, max_height, item_sequence);
            return {r.top_left, affine::extents2d{width, max_height}};
        }


        /// ### Reflow of the items
        /**
         * The expanding item is reflowed on its own after the others so that
         * it can be told how much they have left it. The width it reports is
         * the row's best guess at how much of the offer will be taken up, so a
         * greedy item makes the row ask its parent for the full width and a
         * naturally sized one keeps the row narrow.
         */
        template<std::size_t... I>
        float reflow_others(
                reflow_parameters const &p,
                constrained_type const &space,
                std::index_sequence<I...>) {
            float used = {};
            auto const one = [&](std::size_t const index, auto &item) {
                if (index != expanding) {
                    auto const ex = item.reflow(p, space);
                    elements.write_constraints(index, ex);
                    used += ex.width.value();
                }
            };
            (one(I, std::get<I>(items)), ...);
            return used;
        }
        template<std::size_t... I>
        void reflow_expanding(
                reflow_parameters const &p,
                constrained_type const &offer,
                std::index_sequence<I...>) {
            auto const one = [&](std::size_t const index, auto &item) {
                if (index == expanding) {
                    elements.write_constraints(index, item.reflow(p, offer));
                }
            };
            (one(I, std::get<I>(items)), ...);
        }

        /// ### Move the items, left to right
        /**
         * The comma fold evaluates left to right, so each item is moved
         * knowing how much room the ones before it have actually taken. Only
         * the expanding item's answer is believed -- the others are moved to
         * exactly the width they reflowed to, as they would be in a `row`.
         * Returns the width the row has ended up covering.
         */
        template<std::size_t... I>
        float move_items(
                reflow_parameters const &p,
                affine::rectangle2d const &r,
                auto const &sizes,
                float const offer,
                float const max_height,
                std::index_sequence<I...>) {
            float left = {};
            auto const place = [&](std::size_t const index, auto &item) {
                bool const expands = index == expanding;
                float const width =
                        expands ? offer : sizes[index].width.value();
                affine::rectangle2d const into{
                        r.top_left + affine::point2d{left, {}},
                        affine::extents2d{width, max_height}};
                auto const taken = item.move_to(p, into);
                float const covered = expands ? taken.extents.width : width;
                elements.at(index).position = {
                        affine::point2d{left, {}},
                        affine::extents2d{covered, max_height}};
                left += covered + padding;
            };
            (place(I, std::get<I>(items)), ...);
            return left - padding;
        }

        /// ### The expanding index must be one of the items
        /**
         * Without this an out of range index would silently turn the row into
         * a plain `row` with nothing expanding into the left over space.
         */
        void check_expanding(
                std::source_location const &loc =
                        std::source_location::current()) const {
            if (expanding >= item_count) {
                throw felspar::stdexcept::logic_error{
                        "The expanding index " + std::to_string(expanding)
                                + " is not one of the "
                                + std::to_string(item_count)
                                + " items in the `expandable_row` `" + name()
                                + "`",
                        loc};
            }
        }
    };


    template<typename C>
    expandable_row(std::size_t, C) -> expandable_row<C>;
    template<typename C>
    expandable_row(std::string_view, std::size_t, C) -> expandable_row<C>;
    template<typename C>
    expandable_row(std::size_t, C, float) -> expandable_row<C>;
    template<typename C>
    expandable_row(std::string_view, std::size_t, C, float)
            -> expandable_row<C>;


}
