#pragma once


#include <planet/ui/constrained.hpp>


namespace planet::ui {


    /// ## Padded UI element
    /**
     * This is not a UI element per-se, but rather something that can be used by
     * anything implementing a UI element.
     *
     * See [box.hpp](./box.hpp) for an example of usage.
     */
    struct padding {
        using constrained_type = constrained2d<float>;


        float hpad = {}, vpad = hpad;


        /// ### Remove padding
        /**
         * Use the `remove_padding` functions to take the padding away. This is
         * used when adjusting the outer sizes to what is to be passed to the
         * inner ones.
         */
        constrained_type remove(constrained_type ex) const noexcept {
            ex.width.max(ex.width.max() - 2 * hpad);
            ex.height.max(ex.height.max() - 2 * vpad);
            return ex;
        }
        affine::extents2d remove(affine::extents2d const ex) const noexcept {
            return {ex.width - 2 * hpad, ex.height - 2 * vpad};
        }
        /// ### Add padding
        /**
         * Use the `add` function to calculate the correct sizes to
         * return to the outer element.
         */
    };


}
