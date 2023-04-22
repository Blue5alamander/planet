#pragma once


#include <planet/ui/panel.hpp>


namespace planet::ui {


    template<typename Renderer>
    class widget {
        felspar::coro::eager<> response;

      public:
        widget() = default;
        widget(widget const &) = delete;
        widget(widget &&) = default;
        virtual ~widget() = default;

        widget &operator=(widget const &) = delete;
        widget &operator=(widget &&) = default;

        void add_to(ui::panel &parent) {
            parent.add_child(panel);
            response.post(*this, &widget::behaviour);
            visible = true;
        }

        void draw_within(Renderer &r, affine::rectangle2d const outer) {
            if (visible) { do_draw_within(r, outer); }
        }

      protected:
        ui::panel panel;
        bool visible = false;

        virtual felspar::coro::task<void> behaviour() = 0;
        virtual void do_draw_within(Renderer &r, affine::rectangle2d) = 0;
    };


}
