#pragma once


#include <planet/affine/rectangle2d.hpp>
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


        /**
         * TODO Probably best to follow the padding order that CSS makes use of
         * <https://developer.mozilla.org/en-US/docs/Web/CSS/padding>
         */
        float left = {}, top = left, right = left, bottom = top;


        /// ### Remove padding
        /**
         * Use the `remove_padding` functions to take the padding away. This is
         * used when adjusting the outer sizes to what is to be passed to the
         * inner ones.
         */
        constrained_type remove(constrained_type ex) const noexcept {
            auto const wpad = left + right, hpad = top + bottom;

            ex.width.min(std::max(0.0f, ex.width.min() - wpad));
            ex.width.desire(ex.width.value() - wpad);
            ex.width.max(ex.width.max() - wpad);

            ex.height.min(std::max(0.0f, ex.height.min() - hpad));
            ex.height.desire(ex.height.value() - hpad);
            ex.height.max(ex.height.max() - hpad);

            return ex;
        }
        affine::extents2d remove(affine::extents2d const ex) const noexcept {
            return {ex.width - left - right, ex.height - top - bottom};
        }
        affine::rectangle2d remove(affine::rectangle2d const &r) const noexcept {
            return {r.top_left + affine::point2d{left, top}, remove(r.extents)};
        }
    };


}
