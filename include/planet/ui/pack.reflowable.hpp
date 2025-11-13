#pragma once


#include <planet/tuple.hpp>
#include <planet/ui/layout.hpp>
#include <planet/ui/reflowable.hpp>


namespace planet::ui {


    /// ## Reflowable implementation for tuple based layouts
    template<typename ET, typename... Pack>
    struct pack_reflowable : public reflowable {
        using collection_type = std::tuple<Pack...>;


        /// ### Construction
        explicit pack_reflowable(
                std::string_view const n,
                collection_type c,
                std::source_location const &loc =
                        std::source_location::current())
        : reflowable{n}, items{std::move(c)}, created_loc{loc} {}


        collection_type items;


        /// ### Meta-data
        static constexpr std::size_t item_count = sizeof...(Pack);
        static constexpr auto item_sequence =
                std::make_index_sequence<item_count>{};
        static_assert(item_count > 0, "There must be at least one UI element");


        /// ### Information about the layout of the contained items
        using layout_type = planet::ui::layout<
                std::array<planet::ui::element<ET>, item_count>>;
        using constrained_type = typename layout_type::constrained_type;
        layout_type elements;


        /// ### Draw the contained items
        void draw() {
            exec_all(items, [](auto &item) { item.draw(); });
        }


      protected:
        /// ### Return an array of constraints for all of the included items
        auto items_reflow(reflow_parameters const &p, constrained_type const &c) {
            return items_reflow_sequence(p, c, item_sequence);
        }
        auto items_constraints() { return constraints_sequence(item_sequence); }

        affine::rectangle2d move_sub_elements(
                reflow_parameters const &p,
                affine::rectangle2d const &r) override {
            move_elements(p, r, item_sequence);
            return {r.top_left, constraints().extents()};
        }


      private:
        std::source_location created_loc;

        template<std::size_t... I>
        void move_elements(
                reflow_parameters const &p,
                affine::rectangle2d const &r,
                std::index_sequence<I...>) {
            try {
                auto const tl = r.top_left;
                std::array const pos{elements.at(I).position.value()...};
                (std::get<I>(items).move_to(
                         p, {pos[I].top_left + tl, pos[I].extents}),
                 ...);
            } catch (std::bad_optional_access const &) {
                throw felspar::stdexcept::logic_error{
                        "Element position has not been set for "
                        "`pack_reflowable` id `"
                                + name() + "` created at",
                        created_loc};
            }
        }
        template<std::size_t... I>
        auto items_reflow_sequence(
                reflow_parameters const &p,
                constrained_type const &c,
                std::index_sequence<I...>) {
            (elements.write_constraints(I, std::get<I>(items).reflow(p, c)),
             ...);
            return constraints_sequence(item_sequence);
        }
        template<std::size_t... I>
        auto constraints_sequence(std::index_sequence<I...>) {
            return std::array{elements.at(I).constraints...};
        }
    };


}
