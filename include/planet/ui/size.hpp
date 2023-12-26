#pragma once


#include <planet/affine/extents2d.hpp>
#include <planet/ui/reflowable.hpp>


namespace planet::ui {


    /// ## Draw re-sizeable content at the specified size
    template<typename C>
    struct target_size : public reflowable {
        using content_type = C;
        content_type content;
        affine::extents2d size;

        target_size(content_type c, affine::extents2d const s)
            requires(not std::is_lvalue_reference_v<content_type>)
        : reflowable{"planet::ui::target_size"},
          content{std::move(c)},
          size{s} {}
        target_size(content_type c, affine::extents2d const s)
            requires std::is_lvalue_reference_v<content_type>
        : reflowable{"planet::ui::target_size"}, content{c}, size{s} {}

        affine::extents2d extents(affine::extents2d const &) const noexcept {
            return size;
        }

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

        void move_sub_elements(affine::rectangle2d const &r) override {
            content.move_to({r.top_left, content.constraints().extents()});
        }
    };
    template<typename C>
    target_size(C, affine::extents2d) -> target_size<C>;


}
