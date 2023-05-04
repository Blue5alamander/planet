#pragma once


#include <planet/affine/extents2d.hpp>
#include <planet/ui/reflowable.hpp>


namespace planet::ui {


    /// Draw re-sizeable content at the specified size
    template<typename C>
    struct target_size : public reflowable {
        using content_type = C;
        content_type content;
        affine::extents2d size;

        target_size(content_type c, affine::extents2d const s)
        : content{std::move(c)}, size{s} {}

        affine::extents2d extents(affine::extents2d const &) const noexcept {
            return size;
        }

        template<typename Target>
        void draw_within(Target &t, affine::rectangle2d const bounds) {
            content.draw_within(t, {bounds.top_left, size});
        }

      private:
        constrained_type do_reflow(constrained_type const &) override {
            return constrained_type{size};
        }
    };


}
