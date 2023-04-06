#pragma once


#include <planet/ui/panel.hpp>


namespace planet::ui {


    template<typename Renderer>
    class widget {
        felspar::coro::eager<> response;

      public:
        void add_to(planet::panel &parent) {
            parent.add_child(panel);
            response.post(*this, &widget::behaviour);
            visible = true;
        }

        void draw_within(Renderer &r, affine::rectangle2d const outer) {
            if (visible) { do_draw_within(r, outer); }
        }

      protected:
        planet::panel panel;
        bool visible = false;

        virtual felspar::coro::task<void> behaviour() = 0;
        virtual void do_draw_within(Renderer &r, affine::rectangle2d) = 0;
    };


}
