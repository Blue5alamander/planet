#pragma once


#include <planet/ui/widget.hpp>


namespace planet::widget {


    template<typename Texture>
    struct checkbox final : public ui::widget {
        using constrained_type = widget::constrained_type;


        checkbox(std::string_view const n, Texture on, Texture off, bool &v)
        : widget{n}, on{std::move(on)}, off{std::move(off)}, value{v} {}

        checkbox(checkbox &&cb)
        : widget{std::move(cb)},
          on{std::move(cb.on)},
          off{std::move(cb.off)},
          value{cb.value} {
            if (has_baseplate()) { response.post(behaviour()); }
        }


        Texture on, off;
        bool &value;


      private:
        void do_draw() override {
            if (value) {
                on.draw();
            } else {
                off.draw();
            }
        }

        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &ex) override {
            auto const on_size = on.reflow(p, ex);
            auto const off_size = off.reflow(p, ex);
            typename constrained_type::axis_constrained_type const w{
                    std::max(on_size.width.min(), off_size.width.min()),
                    std::max(on_size.width.value(), off_size.width.value()),
                    std::min(on_size.width.max(), off_size.width.max())};
            typename constrained_type::axis_constrained_type const h{
                    std::max(on_size.height.min(), off_size.height.min()),
                    std::max(on_size.height.value(), off_size.height.value()),
                    std::min(on_size.height.max(), off_size.height.max())};
            return {w, h};
        }
        affine::rectangle2d do_move_sub_elements(
                reflow_parameters const &p,
                affine::rectangle2d const &r) override {
            /// TODO Should be the union of the two returned rectangles
            on.move_to(p, r);
            return off.move_to(p, r);
        }

        felspar::coro::task<void> behaviour() override {
            for (auto clicks =
                         events::identify_clicks(widget::events.mouse.stream());
                 co_await clicks.next();) {
                value = not value;
            }
        }
    };


}
