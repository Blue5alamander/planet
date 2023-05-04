#pragma once


#include <planet/ui/element.hpp>


namespace planet::ui {


    /// ## Reflow controls UI element layouts
    template<typename Renderer, typename V = void>
    struct reflowableX {
        using constrained_type = constrained2d<float>;

        /// ### Dirty calculation
        /**
         * For UI elements that contain sub-elements this must check all
         * sub-elements and return true if any of them do so.
         */
        virtual bool is_dirty() const { return not layout.has_value(); }

        /// ### Calculate position
        /**
         * Calculates the correct position for this element given the
         * constraints and returns a concrete position that the element will be
         * drawn in given the constraints.
         */
        // virtual void reflow(constrained_type const &) = 0;

        /// ### Calculated position
        std::optional<element<V, constrained_type::value_type>> layout;

        /// ### Draw at the calculated position
        // virtual void draw(Renderer &) const = 0;
    };


}
