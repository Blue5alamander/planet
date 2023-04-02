#pragma once


#include <planet/affine2d.hpp>


namespace planet::ui {


    /// ## Gravity direction
    /**
     * Combine these flags to determine how an element fits inside the given
     * space
     *
     * When none are specified then the content is stretched to fill the space.
     * Individual flags represent a pull in that direction and will turn
     * stretching off. Two opposing flags will causing centring along that axis.
     */
    enum gravity : unsigned char {
        fill = 0,
        left = 1,
        right = 2,
        top = 4,
        bottom = 8
    };
    inline gravity operator|(gravity l, gravity r) {
        return static_cast<gravity>(
                static_cast<unsigned char>(l)
                bitor static_cast<unsigned char>(r));
    }

    /// ## Rectangle positioning
    /**
     * Calculate the extent within the outer extent that the inner will have
     * based on the gravity passed in
     */
    affine::rectangle2d
            within(gravity,
                   affine::rectangle2d const &outer,
                   affine::extents2d const &inner);

    /// ## Box wrapper
    /**
     * A container for another element. The box itself is not drawn. The
     * `gravity` can be used to control how the content is positioned within the
     * box.
     */
    template<typename C>
    struct box {
        /// What is inside the box
        using content_type = C;
        content_type content;
        /// The size of the box in its container's coordinate system
        gravity inner = {
                gravity::left | gravity::right | gravity::top
                | gravity::bottom};
        /// The amount of padding to be added around the content.
        float hpadding = {}, vpadding = {};

        box() {}
        explicit box(content_type c) : content{std::move(c)} {}
        box(content_type c,
            gravity const g,
            float const hp = {},
            float const vp = {})
        : content{std::move(c)}, inner{g}, hpadding{hp}, vpadding{vp} {}

        /// ### Calculate the extents of the box
        affine::extents2d extents(affine::extents2d const &ex) {
            return add_padding(content.extents(remove_padding(ex)));
        }

        /// ### Drawing the box content
        /**
         * Draw the content within the area outlined by the top left and bottom
         * right corners passed in. All calculations are done in the screen
         * space co-ordinate system
         */
        template<typename Target>
        void draw_within(Target &t, affine::rectangle2d const outer) {
            auto const area = within(
                    inner,
                    {outer.top_left + affine::point2d{hpadding, vpadding},
                     remove_padding(outer.extents)},
                    content.extents(remove_padding(outer.extents)));
            content.draw_within(t, area);
        }

      private:
        affine::extents2d
                remove_padding(affine::extents2d const ex) const noexcept {
            return {ex.width - 2 * hpadding, ex.height - 2 * vpadding};
        }
        affine::extents2d
                add_padding(affine::extents2d const ex) const noexcept {
            return {ex.width + 2 * hpadding, ex.height + 2 * vpadding};
        }
    };


}
