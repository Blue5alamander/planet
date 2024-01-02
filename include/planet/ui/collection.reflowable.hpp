#pragma once


#include <planet/ui/concepts.hpp>
#include <planet/ui/layout.hpp>
#include <planet/ui/reflowable.hpp>


namespace planet::ui {


    /// ## Reflowable implementation for general layouts
    /**
     * This implements common aspects of reflowable code for any container that
     * uses types like `std::array` or `std::vector` to store sub-elements.
     */
    template<typename CT, typename ET>
    struct collection_reflowable : public reflowable {
        using collection_type = CT;
        using content_type = typename CT::value_type;

        using layout_type = planet::ui::layout_for<collection_type>;
        using constrained_type = typename layout_type::constrained_type;
        using element_type = typename layout_type::element_type;


        explicit collection_reflowable(std::string_view const n)
        : reflowable{n} {}
        explicit collection_reflowable(
                std::string_view const n, collection_type c)
        : reflowable{n}, items{std::move(c)} {}


        collection_type items;
        layout_type elements;


        void draw() {
            for (auto &item : items) { item.draw(); }
        }


        /// ### Visibility
        void visible(bool const v)
            requires(visibility<content_type>)
        {
            for (auto &i : items) { i.visible(v); }
        }
        bool is_visible() const noexcept
            requires(visibility<content_type>)
        {
            bool v = false;
            for (auto const &i : items) { v = v or i.is_visible(); }
            return v;
        }


      protected:
        affine::rectangle2d
                move_sub_elements(affine::rectangle2d const &r) override {
            for (std::size_t index{}; auto &item : items) {
                auto const &epos = elements.at(index).position.value();
                affine::rectangle2d const np{
                        r.top_left + epos.top_left, epos.extents};
                item.move_to(np);
                ++index;
            }
            return {r.top_left, constraints().extents()};
        }
    };


}
