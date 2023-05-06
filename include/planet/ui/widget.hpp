#pragma once


#include <planet/events/bus.hpp>
#include <planet/ui/reflowable.hpp>
#include <planet/ui/panel.hpp>


namespace planet::ui {


    /// ## User interface widget
    template<typename Renderer>
    class widget : public reflowable {
        friend class baseplate<Renderer>;

        felspar::coro::eager<> response;

      public:
        widget() = default;
        widget(widget const &) = delete;
        widget(widget &&w)
        : response{std::move(w.response)},
          baseplate{std::exchange(w.baseplate, nullptr)} {}
        widget(std::string_view const n) : reflowable{n} {}
        virtual ~widget() { deregister(baseplate, this); }

        widget &operator=(widget const &) = delete;
        widget &operator=(widget &&);

        /// ### Events going to this widget
        events::bus events;

        /// ### Add a widget to a base plate so it can receive events
        virtual void
                add_to(ui::baseplate<Renderer> &,
                       ui::panel &parent,
                       float z_layer = {});

        /// ### Draw the widget
        void draw_within(Renderer &r, affine::rectangle2d const outer) {
            if (visible) { do_draw_within(r, outer); }
        }
        void draw(Renderer &r) {
            if (visible) { do_draw(r); }
        }

        /// ### Check if a point in screen space is within the widget
        virtual bool is_within(affine::point2d const &p) const {
            return panel.contains(p);
        }
        /// ### Return true if this widget can take focus
        virtual bool wants_focus() const { return visible; }

      protected:
        ui::panel panel;
        ui::baseplate<Renderer> *baseplate = nullptr;
        static void deregister(ui::baseplate<Renderer> *, widget *);
        bool visible = false;

        virtual felspar::coro::task<void> behaviour() = 0;
        virtual void do_draw_within(Renderer &r, affine::rectangle2d) = 0;
        virtual void do_draw(Renderer &r) = 0;
        virtual void do_move_sub_elements(affine::rectangle2d const &r) = 0;

        /// ### Move the widget's panel
        void move_sub_elements(affine::rectangle2d const &r) override {
            panel.move_to(r);
            do_move_sub_elements(r);
        }
    };


}
