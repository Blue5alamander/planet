#pragma once


#include <planet/affine/rectangle2d.hpp>
#include <planet/ui/constrained.hpp>
#include <planet/ui/reflowable.hpp>


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
        using reflow_parameters = reflowable::reflow_parameters;


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


        /// ### Layout helpers
        /**
         * A UI element takes on padding by lifting the body of its `do_reflow`
         * or `move_sub_elements` into the lambda. The lambda is handed the
         * reflow parameters unchanged together with the space that is left once
         * the padding has been taken off, so whichever helper is used it lays
         * out as though the padding were not there.
         *
         * `reflow` serves both: the element asks for the space it needs plus
         * the padding, so its container sets the room aside. What differs is
         * the area the element goes on to claim, and the name of the `move_to`
         * used says which is meant:
         *
         * * `move_to_with_outer_padding` reports back the rectangle the lambda
         *   laid out in, so the padded ring is owned by nobody. For a widget it
         *   is dead space -- pointer events over it fall through to whatever is
         *   underneath rather than reaching the widget.
         * * `move_to_with_inner_margin` reports that rectangle with the padding
         *   put back around it, so the element covers its own margin. A widget
         *   stays sensitive right across it.
         */
        template<typename Lambda>
        constrained_type
                reflow(reflow_parameters const &p,
                       constrained_type const &ex,
                       Lambda &&l) const {
            auto const inner = l(p, remove_from(ex));
            auto const wpad = left + right, hpad = top + bottom;
            return {{inner.width.value() + wpad,
                     std::max(inner.width.min() + wpad, ex.width.min()),
                     ex.width.max()},
                    {inner.height.value() + hpad,
                     std::max(inner.height.min() + hpad, ex.height.min()),
                     ex.height.max()}};
        }

        template<typename Lambda>
        affine::rectangle2d move_to_with_outer_padding(
                reflow_parameters const &p,
                affine::rectangle2d const &r,
                Lambda &&l) const {
            return l(p, remove_from(r));
        }

        template<typename Lambda>
        affine::rectangle2d move_to_with_inner_margin(
                reflow_parameters const &p,
                affine::rectangle2d const &r,
                Lambda &&l) const {
            return add_to(l(p, remove_from(r)));
        }


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


        /// ### Add padding
        /**
         * The inverse of `remove_from`, taking an inner size or position back
         * out to the outer one that surrounds it.
         */
        affine::extents2d add_to(affine::extents2d const ex) const noexcept {
            return {ex.width + left + right, ex.height + top + bottom};
        }
        affine::rectangle2d add_to(affine::rectangle2d const &r) const noexcept {
            return {r.top_left - affine::point2d{left, top}, add_to(r.extents)};
        }
    };


}
