#pragma once


#include <planet/ui/reflowable.hpp>


namespace planet::ui {


    /// ## Cap how wide content may be laid out
    /**
     * The content is reflowed into a narrowed constraint: the maximum width is
     * pulled in to `width`, and the desired width comes down with it when it
     * was larger. Nothing else is touched -- the minimum width and all three
     * heights pass through -- so content that wants less than the cap still
     * reports the smaller size, and content that re-flows (prose in a
     * `breakable_row`) grows taller rather than wider.
     *
     * The cap never goes below the minimum width being asked for, because a
     * maximum under the minimum describes a width that nothing can satisfy. A
     * container that insists on more than `width` gets what it insists on.
     */
    template<typename C>
    struct max_width final : public reflowable {
        using content_type = C;


        max_width(content_type c, float const w)
            requires(not std::is_lvalue_reference_v<content_type>)
        : reflowable{"planet::ui::max_width"},
          content{std::move(c)},
          width{w} {}
        max_width(content_type c, float const w)
            requires std::is_lvalue_reference_v<content_type>
        : reflowable{"planet::ui::max_width"}, content{c}, width{w} {}


        content_type content;
        /// ### The widest the content may become
        float width;


        void draw() { content.draw(); }


      private:
        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &c) override {
            auto const cap =
                    std::max(c.width.min(), std::min(c.width.max(), width));
            auto narrowed = c;
            if (narrowed.width.value() > cap) { narrowed.width.desire(cap); }
            narrowed.width.max(cap);
            return content.reflow(p, narrowed);
        }

        affine::rectangle2d move_sub_elements(
                reflow_parameters const &p,
                affine::rectangle2d const &r) override {
            affine::rectangle2d const area{
                    r.top_left,
                    affine::extents2d{
                            std::min(r.extents.width, width), r.extents.height}};
            content.move_to(p, area);
            return area;
        }
    };


    template<typename C>
    max_width(C, float) -> max_width<C>;


}
