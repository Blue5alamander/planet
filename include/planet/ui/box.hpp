#pragma once


#include <planet/ui/gravity.hpp>
#include <planet/ui/helpers.hpp>
#include <planet/ui/reflowable.hpp>
#include <planet/ui/padding.hpp>


namespace planet::ui {


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
        box(content_type c)
        : reflowable{"planet::ui::box"}, content{std::move(c)} {}
        box(content_type c, float const hp, float const vp)
        : reflowable{"planet::ui::box"},
          content{std::move(c)},
          padding{hp, vp} {}
        box(content_type c, gravity const g, float const p = {})
        : reflowable{"planet::ui::box"},
          content{std::move(c)},
          inner{g},
          padding{p} {}
        box(std::string_view const n, content_type c)
        : reflowable{n}, content{std::move(c)} {}

        /// ### Drawing the box content
        void draw() { content.draw(); }

      private:
        constrained_type do_reflow(constrained_type const &ex) override {
            return add(content.reflow(padding.remove(ex)), ex);
        }
        affine::rectangle2d
                move_sub_elements(affine::rectangle2d const &outer) override {
            auto const inner_size = content.constraints().extents();
            auto const area = within(inner, padding.remove(outer), inner_size);
            content.move_to(area);
            return outer;
        }
        constrained_type
                add(constrained_type const &inner,
                    constrained_type const &outer) const noexcept {
            auto const min_width =
                    inner.width.min() + padding.left + padding.right;
            auto const min_height =
                    inner.height.min() + padding.top + padding.bottom;
            return {{outer.width.value(),
                     std::max(min_width, outer.width.min()), outer.width.max()},
                    {outer.height.value(),
                     std::max(min_height, outer.height.min()),
                     outer.height.max()}};
        }
    };


}
