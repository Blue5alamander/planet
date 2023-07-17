#pragma once


#include <planet/affine2d.hpp>
#include <planet/ui/helpers.hpp>
#include <planet/ui/reflowable.hpp>
#include <planet/ui/padding.hpp>


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
    struct box final : public reflowable {
        /// What is inside the box
        using content_type = C;
        content_type content;
        /// The size of the box in its container's coordinate system
        gravity inner = {
                gravity::left | gravity::right | gravity::top
                | gravity::bottom};
        /// The amount of padding to be added around the content.
        ui::padding padding = {};

        box() : reflowable{"planet::ui::box"} {}
        explicit box(content_type c)
        : reflowable{"planet::ui::box"}, content{std::move(c)} {}
        explicit box(content_type c, float const hp, float const vp)
        : reflowable{"planet::ui::box"}, content{std::move(c)}, padding{hp, vp} {}
        explicit box(content_type c, gravity const g, float const p = {})
        : reflowable{"planet::ui::box"},
          content{std::move(c)},
          inner{g},
          padding{p} {}
        explicit box(std::string_view const n, content_type c)
        : reflowable{n}, content{std::move(c)} {}

        /// ### Drawing the box content
        template<typename Renderer>
        void
                draw(Renderer &r,
                     felspar::source_location const &loc =
                             felspar::source_location::current()) {
            detail::draw(r, content, loc);
        }

      private:
        constrained_type do_reflow(constrained_type const &ex) override {
            return padding.add(content.reflow(padding.remove(ex)), ex);
        }
        void move_sub_elements(affine::rectangle2d const &outer) override {
            affine::point2d const offset{padding.hpad, padding.vpad};
            auto const inner_size = content.constraints().extents();
            auto const area = within(
                    inner,
                    {outer.top_left + offset, padding.remove(outer.extents)},
                    inner_size);
            content.move_to(area);
        }
    };


}
