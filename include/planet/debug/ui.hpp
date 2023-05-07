#pragma once


#include <planet/affine/rectangle2d.hpp>
#include <planet/ui/reflowable.hpp>


namespace planet::debug {


    /// UI element
    struct fixed_element : public ui::reflowable {
        affine::extents2d size;

        fixed_element(affine::extents2d const s)
        : reflowable{"planet::debug::fixed_element"}, size{s} {}

        affine::extents2d extents(affine::extents2d) const noexcept {
            return size;
        }

        void draw_within(std::ostream &, affine::rectangle2d);
        void draw(std::ostream &);

      private:
        constrained_type do_reflow(constrained_type const &) override {
            return constrained_type{size};
        }
        void move_sub_elements(affine::rectangle2d const &) override {}
    };


}
