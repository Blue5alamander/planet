#pragma once


#include <planet/affine/rectangle2d.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/reflowable.hpp>
#include <planet/ui/widget.hpp>


namespace planet::debug {


    /// ## Fixed size UI element
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


    /// ## Button
    struct button final : public ui::widget<std::ostream &> {
        using superclass = ui::widget<std::ostream &>;

        button() : superclass{"planet::debug::button"} {}

        constrained_type do_reflow(constrained_type const &c) { return c; }
        felspar::coro::task<void> behaviour() { co_return; }
        void do_draw(std::ostream &os) {
            os << name() << " do_draw @ " << position() << '\n';
        }
        void do_move_sub_elements(affine::rectangle2d const &) {}
    };


}
