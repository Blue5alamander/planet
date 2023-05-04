#pragma once


#include <planet/ui/element.hpp>


namespace planet::ui {


    /// ## Reflow controls UI element layouts
    template<typename Renderer, typename V = void>
    struct reflowableX {
        using constrained_type = constrained2d<float>;

        /// ### Dirty calculation

        /// #### Set this item as dirty
        void set_dirty() { reflowed_size = {}; }

        /// #### Check item dirtyness
        /**
         * For UI elements that contain sub-elements this must check all
         * sub-elements and return true if any of them do so.
         */
        virtual bool is_dirty() const { return not reflowed_size.has_value(); }

        /// ### Calculated position
        /// `reflow` must have been called before this size is required
        affine::extents2d const &size() { return reflowed_size.value(); }

        /// ### Calculate position
        /**
         * Calculates the correct position for this element given the
         * constraints and returns a concrete position that the element will be
         * drawn in given the constraints.
         *
         * The constraint passed in represents the possible extents of the item,
         * and the returned constraint gives minimums and maximums for the
         * layout. In general it isn't guaranteed that the input constraint can
         * be met, and parent items need to take this into account or layouts
         * will break in weird and wonderful ways.
         *
         * Calling this will also make the `size` member available so that other
         * code can be aware of the concrete size that has been chosen to draw
         * this item. The code that performs the draw will also need to be able
         * to make use of this calculated value to correctly draw the item.
         */
        constrained_type reflow(constrained_type const &ex) {
            auto const r = do_reflow(ex);
            reflowed_size =
                    affine::extents2d{r.width.value(), r.height.value()};
            return r;
        }

        /// ### Draw at the calculated position
        // virtual void draw(Renderer &, affine::point2d const &offset) const = 0;

      protected:
        /// ### Reflow implementation
        virtual constrained_type do_reflow(constrained_type const &) = 0;

      private:
        std::optional<affine::extents2d> reflowed_size;
    };


}
