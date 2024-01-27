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


        /// ### Manually set the state

        /// #### Enable
        void enable(bool const v) noexcept { m_enabled = v; }


        /// ### Add a widget to a base plate so it can receive events
        /// This will also set the widget to visible.
        virtual void add_to(ui::baseplate &, ui::panel &parent);
        void add_to(ui::baseplate &bp) { add_to(bp, bp.pixels); }


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


      protected:
        ui::panel panel;
        ui::baseplate *baseplate = nullptr;
        static void deregister(ui::baseplate *, widget *);
        bool m_enabled = true;

        virtual felspar::coro::task<void> behaviour() = 0;
        virtual void do_draw() = 0;


        /// ### Move the widget's panel
        affine::rectangle2d
                move_sub_elements(affine::rectangle2d const &r) override {
            auto const se = do_move_sub_elements(r);
            panel.move_to(se);
            return se;
        }
        virtual affine::rectangle2d
                do_move_sub_elements(affine::rectangle2d const &) = 0;

        felspar::coro::eager<> response;
    };


}
