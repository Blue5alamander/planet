#pragma once


#include <planet/affine/rectangle2d.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/baseplate.hpp>
#include <planet/ui/reflowable.hpp>
#include <planet/ui/widget.hpp>


namespace planet::debug {


    /// ## Generic printable component
    template<typename AS>
    struct printable final : public AS {
        using AS::AS;

      private:
        void do_draw(std::ostream &os) {
            os << AS::name() << " do_draw @ " << AS::position() << '\n';
        }
    };


    /// ## Fixed size UI element
    struct fixed_element final : public ui::reflowable {
        affine::extents2d size;

        fixed_element(affine::extents2d const s)
        : reflowable{"planet::debug::fixed_element"}, size{s} {}

        void draw(std::ostream &os) {
            os << name() << " draw @ " << position() << '\n';
        }

      private:
        constrained_type do_reflow(constrained_type const &) override {
            return constrained_type{size};
        }
        void move_sub_elements(affine::rectangle2d const &) override {}
    };


    /// ## Button
    template<typename C = void>
    struct button final : public ui::widget<std::ostream &> {
        using superclass = ui::widget<std::ostream &>;
        using content_type = C;

        button(content_type c)
        : superclass{"planet::debug::button"}, content{std::move(c)} {}

        content_type content;

        /// ### The number of times the button has been pressed
        std::size_t clicks = {};

        constrained_type do_reflow(constrained_type const &c) {
            return content.reflow(c);
        }
        felspar::coro::task<void> behaviour() {
            for (auto mc = events::identify_clicks(
                         baseplate->mouse_settings, events.mouse.stream());
                 auto click = co_await mc.next();) {
                ++clicks;
            }
        }
        void do_draw(std::ostream &os) { content.draw(os); }
        void do_move_sub_elements(affine::rectangle2d const &c) {
            content.move_to(c);
        }
    };
    template<>
    struct button<void> final : public ui::widget<std::ostream &> {
        using superclass = ui::widget<std::ostream &>;

        button() : superclass{"planet::debug::button"} {}

        /// ### The number of times the button has been pressed
        std::size_t clicks = {};

        constrained_type do_reflow(constrained_type const &c) { return c; }
        felspar::coro::task<void> behaviour() {
            for (auto mc = events::identify_clicks(
                         baseplate->mouse_settings, events.mouse.stream());
                 auto click = co_await mc.next();) {
                ++clicks;
            }
        }
        void do_draw(std::ostream &os) {
            os << name() << " do_draw @ " << position() << '\n';
        }
        void do_move_sub_elements(affine::rectangle2d const &) {}
    };


}
