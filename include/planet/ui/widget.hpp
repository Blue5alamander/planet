#pragma once


#include <planet/events/queue.hpp>
#include <planet/ui/reflowable.hpp>
#include <planet/ui/panel.hpp>


namespace planet::ui {


    /// ## User interface widget
    template<typename Renderer>
    class widget : public reflowable {
        friend class baseplate<Renderer>;

      public:
        /// ### Construction
        /**
         * TODO Have the widget take a `baseplate` in the constructor, which is
         * then the one that the widget is to appear on.
         */
        widget(widget const &) = delete;
        widget(widget &&w)
        : reflowable{std::move(w)},
          events{std::move(w.events)},
          baseplate{std::exchange(w.baseplate, nullptr)},
          visible{w.visible} {
            if (baseplate) {
                /**
                 * If a widget is moved when the baseplate is active then it's
                 * likely that a `response` coroutine should also be running.
                 * Because the `behaviour` is abstract this coroutine must be
                 * re-started in a sub-class.
                 */
                w.response.destroy();
                baseplate->update_ptr(&w, this);
            }
        }
        widget(std::string_view const n, float const z = {})
        : reflowable{n}, z_layer{z} {}
        virtual ~widget() { deregister(baseplate, this); }

        widget &operator=(widget const &) = delete;
        /// TODO We could allow move assignment
        widget &operator=(widget &&) = delete;


        /// ### Events going to this widget
        events::queue events;


        /// ### Z layer
        /**
         * Widgets with higher z layer values logically appears on top of those
         * with lower ones. This value is used for event routing to determine
         * which of a number of overlapping widgets actually get the soft focus
         * events.
         */
        float z_layer = {};


        /// ### Add a widget to a base plate so it can receive events
        /**
         * This member is implemented near the bottom of
         * [`baseplate.hpp`](./baseplate.hpp).
         */
        virtual void add_to(ui::baseplate<Renderer> &, ui::panel &parent);
        void add_to(ui::baseplate<Renderer> &bp) { add_to(bp, bp.pixels); }


        /// ### Draw the widget
        void draw() {
            if (visible) { do_draw(); }
        }


        /// ### Meta-data

        /// #### Check if a point in global space is within the widget
        virtual bool contains_global_coordinate(
                affine::point2d const &p,
                felspar::source_location const &loc =
                        felspar::source_location::current()) const {
            return position(loc).contains(p);
        }
        /// #### Will we try to draw the widget
        bool is_visible() const noexcept { return visible; }


        /// ### Return true if this widget can take focus
        virtual bool wants_focus() const { return visible; }


      protected:
        ui::panel panel;
        ui::baseplate<Renderer> *baseplate = nullptr;
        static void deregister(ui::baseplate<Renderer> *, widget *);
        bool visible = false;

        virtual felspar::coro::task<void> behaviour() = 0;
        virtual void do_draw() = 0;


        /// ### Move the widget's panel
        void move_sub_elements(affine::rectangle2d const &r) override {
            /// TODO Panel should be moving within the parent's coordinate
            /// space, not the global one
            panel.move_to(r);
            do_move_sub_elements(r);
        }
        virtual void do_move_sub_elements(affine::rectangle2d const &) = 0;

        felspar::coro::eager<> response;
    };


}
