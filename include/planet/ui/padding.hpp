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


        /// ### The padding on each side
        float top = {}, right = top, bottom = top, left = right;
        /**
         * The sides are declared in the order used by the [CSS `padding`
         * shorthand](https://developer.mozilla.org/en-US/docs/Web/CSS/padding),
         * and each side defaults to the one it mirrors, so brace
         * initialisation follows the same rules that CSS does:
         *
         * * `{a}` -- every side
         * * `{a, b}` -- vertical, horizontal
         * * `{a, b, c}` -- top, horizontal, bottom
         * * `{a, b, c, d}` -- top, right, bottom, left
         *
         * The mirroring applies to designated initialisers as well, so
         * `{.top = v, .right = h}` is the two value shorthand with its sides
         * named. Naming `.left` instead of `.right` would not be: `right`
         * mirrors `top` and would take the vertical padding.
         */


        /// ### Remove padding
        /**
         * Use the `remove_padding` functions to take the padding away. This is
         * used when adjusting the outer sizes to what is to be passed to the
         * inner ones.
         */
        constrained_type remove_from(constrained_type ex) const noexcept {
            auto const wpad = left + right, hpad = top + bottom;

            ex.width.min(std::max(0.0f, ex.width.min() - wpad));
            ex.width.desire(ex.width.value() - wpad);
            ex.width.max(ex.width.max() - wpad);

            ex.height.min(std::max(0.0f, ex.height.min() - hpad));
            ex.height.desire(ex.height.value() - hpad);
            ex.height.max(ex.height.max() - hpad);

            return ex;
        }
        affine::extents2d remove_from(float const ex) const noexcept {
            return remove_from(affine::extents2d{ex, ex});
        }
        affine::extents2d
                remove_from(affine::extents2d const ex) const noexcept {
            return {ex.width - left - right, ex.height - top - bottom};
        }
        affine::rectangle2d
                remove_from(affine::rectangle2d const &r) const noexcept {
            return {r.top_left + affine::point2d{left, top},
                    remove_from(r.extents)};
        }
    };


}
