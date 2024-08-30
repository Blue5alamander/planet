#pragma once


#include <planet/affine/extents2d.hpp>
#include <planet/ui/reflowable.hpp>


namespace planet::ui {


    /// ## Draw re-sizeable content at the specified size
    template<typename C>
    struct target_size final : public reflowable {
        using content_type = C;


        target_size(content_type c, affine::extents2d const s)
            requires(not std::is_lvalue_reference_v<content_type>)
        : reflowable{"planet::ui::target_size"},
          content{std::move(c)},
          size{s} {}
        target_size(content_type c, affine::extents2d const s)
            requires std::is_lvalue_reference_v<content_type>
        : reflowable{"planet::ui::target_size"}, content{c}, size{s} {}


        content_type content;
        affine::extents2d size;
        /// TODO Add `gravity` like box has


        affine::extents2d extents(affine::extents2d const &) const noexcept {
            return size;
        }
        /// TODO This looks like it should go
        template<typename Target>
        void draw_within(Target &t, affine::rectangle2d const bounds) {
            content.draw_within(t, {bounds.top_left, size});
        }
        void draw() { content.draw(); }


      private:
        constrained_type do_reflow(constrained_type const &) override {
            constrained_type const csize{size};
            content.reflow(csize);
            return csize;
        }

        affine::rectangle2d
                move_sub_elements(affine::rectangle2d const &r) override {
            content.move_to({r.top_left, size});
            return {r.top_left, size};
        }
    };

    template<typename C>
    target_size(C, affine::extents2d) -> target_size<C>;


}
