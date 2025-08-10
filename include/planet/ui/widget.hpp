#pragma once


#include <planet/events/queue.hpp>
#include <planet/ui/baseplate.hpp>
#include <planet/ui/reflowable.hpp>
#include <planet/ui/panel.hpp>


namespace planet::ui {


    /// ## User interface widget
    class widget : public reflowable {
        friend class baseplate;


      public:
        /// ### Construction
        /**
         * TODO Have the widget take a `baseplate` in the constructor, which is
         * then the one that the widget is to appear on.
         *
         * Invisible widgets are not drawn. The widget will default to invisible
         * until it is added to a baseplate.
         *
         * Enabled widgets will accept input from the baseplate. Widgets are by
         * default enabled.
         *
         * For the widget to respond to input it must be added to a baseplate
         * using `add_to`.
         *
         * A common bug in sub-classes is to forget that the move constructor
         * needs to `post` the `behaviour` back into the `response`.
         */
        widget(widget const &) = delete;
        widget(widget &&w)
        : reflowable{std::move(w)},
          events{std::move(w.events)},
          baseplate{std::exchange(w.baseplate, nullptr)},
          m_enabled{w.m_enabled} {
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


        /// ### The panel describing the position
        ui::panel panel;


        /// ### Manually set the state

        /// #### Enable
        void enable(bool const v = true) noexcept { m_enabled = v; }


        /// ### Add a widget to a base plate so it can receive events
        /// This will also set the widget to visible.
        virtual void add_to(ui::baseplate &, ui::panel &parent);
        void add_to(ui::baseplate &bp) { add_to(bp, bp.pixels); }
        /// #### Add to another widget
        void add_to(widget &w) {
            if (not w.baseplate) { throw_invalid_add_to_target(); }
            add_to(*w.baseplate, w.panel);
            z_layer = w.z_layer + 1;
        }


        /// ### Draw the widget
        void draw() {
            do_draw();
            if (baseplate) { baseplate->add(this); }
        }


        /// ### Meta-data

        /// #### Whether the widget will accept input
        bool is_enabled() const noexcept { return m_enabled; }


        /// ### Return true if this widget can take focus
        virtual bool wants_focus() const noexcept { return m_enabled; }

        /// #### Turn hard focus on and off
        void hard_focus_on() { baseplate->hard_focus_on(this); }
        void hard_focus_off() { baseplate->hard_focus_off(this); }


      protected:
        /// ### Hover duration
        /**
         * How long the widget has had the mouse hovered over it. The time
         * counts up even if the widget is overlapped by other widgets, but is
         * reset once the mouse leaves the widget.
         */
        virtual void hover(std::chrono::steady_clock::duration) {}


        /// ### Return true if the widget is connected to a baseplate
        bool has_baseplate() const noexcept { return baseplate != nullptr; }


        /// ### Remove a widget from the baseplate
        /**
         * By default widgets are removed from the baseplate at the beginning of
         * the render cycle and then re-added when draw.
         */
        static void deregister(ui::baseplate *, widget *);


        /// ### Required implementation
        virtual felspar::coro::task<void> behaviour() = 0;
        virtual void do_draw() = 0;


        /// ### Hover notification

        /// ### Move the widget's panel
        affine::rectangle2d move_sub_elements(
                reflow_parameters const &p,
                affine::rectangle2d const &r) override {
            auto const se = do_move_sub_elements(p, r);
            panel.move_to(se);
            return se;
        }
        virtual affine::rectangle2d do_move_sub_elements(
                reflow_parameters const &p, affine::rectangle2d const &) = 0;

        /**
         * ### The coroutine for the behaviour
         *
         * The `behaviour` method is run within `response`, and is started by
         * the `add_to` method after the baseplate has been set. (Final)
         * sub-classes will need to post a new behaviour after being moved.
         */
        felspar::coro::eager<> response;


      private:
        std::chrono::steady_clock::duration hover_time = {};
        ui::baseplate *baseplate = nullptr;
        bool m_enabled = true;
        [[noreturn]] void throw_invalid_add_to_target();
    };


}
