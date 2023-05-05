#pragma once


#include <planet/ui/element.hpp>


namespace planet::ui {


    /// ## Reflow UI element layout
    struct reflowable {
        using constrained_type = constrained2d<float>;

        /// ### Dirty calculation

        /// #### Set this item as dirty
        void set_dirty() { position = {}; }

        /// #### Check item dirtyness
        /**
         * For UI elements that contain sub-elements this must check all
         * sub-elements and return true if any of them do so.
         */
        virtual bool is_dirty() const { return not position.has_value(); }

        /// ### Calculated position
        /// `reflow` must have been called before this size is required
        affine::extents2d const &size() { return position.value().extents; }

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
         * After calling `reflow` on all children the parent element must
         * determine their locations. Once done the parent element calls
         * `move_to` on each child to inform it of where to draw.
         */
        constrained_type reflow(constrained_type const &ex) {
            return do_reflow(ex);
        }

        /// ### Move the widget to a new position
        /**
         * For widgets (or other UI items that have a `panel`) the parent must
         * tell the child what the new position is going to be.
         */
        virtual void move_to(affine::rectangle2d const &r) { position = r; }

      protected:
        /// ### Reflow implementation
        virtual constrained_type do_reflow(constrained_type const &) = 0;
        std::optional<affine::rectangle2d> position;
    };


}
