#pragma once


#include <planet/affine2d.hpp>


namespace planet::ui {


    /// Combine these to determine how an element fits inside the given space
    /**
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

    /// Calculate the extent within the outer extent that the inner will have
    /// based on the gravity passed in
    affine::rectangle
            within(gravity,
                   affine::rectangle const &outer,
                   affine::extents2d const &inner);

    /// A container for another element
    template<typename C>
    struct box {
        /// What is inside the box
        using content_type = C;
        content_type content;
        /// The size of the box in its container's coordinate system
        gravity inner;
        /// The amount of padding to be added around the content.
        float hpadding = {}, vpadding = {};

        explicit box(
                content_type c,
                gravity const g = gravity::left | gravity::right | gravity::top
                        | gravity::bottom,
                float const hp = {},
                float const vp = {})
        : content{std::move(c)}, inner{g}, hpadding{hp}, vpadding{vp} {}

        /// Calculate the extents of the box
        affine::extents2d extents(affine::extents2d const &ex) const {
            auto const inner = content.extents(
                    {ex.width - 2 * hpadding, ex.height - 2 * vpadding});
            return {inner.width + 2 * hpadding, inner.height + 2 * vpadding};
        }

        /// Draw the content within the area outlined by the top left and bottom
        /// right corners passed in. All calculations are done in the screen
        /// space co-ordinate system
        template<typename Target>
        void draw_within(Target &t, affine::rectangle const outer) {
            auto const area = within(inner, outer, extents(outer.extents));
            content.draw_within(t, area);
        }
    };


}
