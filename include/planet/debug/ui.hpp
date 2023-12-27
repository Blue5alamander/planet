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


        printable(std::ostream &s) : os{&s} {}


        std::ostream *os;


      private:
        void do_draw() override {
            *os << AS::name() << " do_draw @ " << AS::position() << '\n';
        }
    };


    /// ## Fixed size UI element
    struct fixed_element final : public ui::reflowable {
        std::ostream *os;
        affine::extents2d size;


        fixed_element(std::ostream &ss, affine::extents2d const sz)
        : reflowable{"planet::debug::fixed_element"}, os{&ss}, size{sz} {}


        void draw() { *os << name() << " draw @ " << position() << '\n'; }


      private:
        constrained_type do_reflow(constrained_type const &) override {
            return constrained_type{size};
        }
        affine::rectangle2d
                move_sub_elements(affine::rectangle2d const &r) override {
            return {r.top_left, size};
        }
    };


    /// ## Button
    template<typename C = void>
    struct button final : public ui::widget {
        using content_type = C;


        button(content_type c)
        : ui::widget{"planet::debug::button"}, content{std::move(c)} {}


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
        void do_draw() { content.draw(); }
        affine::rectangle2d do_move_sub_elements(affine::rectangle2d const &c) {
            return content.move_to(c);
        }
    };
    template<>
    struct button<void> final : public ui::widget {
        button(std::ostream &o) : ui::widget{"planet::debug::button"}, os{&o} {}


        std::ostream *os;
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
        void do_draw() { *os << name() << " do_draw @ " << position() << '\n'; }
        affine::rectangle2d do_move_sub_elements(affine::rectangle2d const &r) {
            return r;
        }
    };


}
