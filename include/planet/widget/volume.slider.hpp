#pragma once


#include <planet/audio/channel.hpp>
#include <planet/ui/box.hpp>
#include <planet/ui/range.hpp>


namespace planet::widget::volume {


    /// ## Volume slider
    template<typename Background, typename Draggable>
    struct slider : public planet::ui::range_updater {
        using range_type =
                planet::ui::range<Background, planet::ui::draggable<Draggable>>;
        using constrained_type = typename range_type::constrained_type;
        using reflow_parameters = typename range_type::reflow_parameters;


        slider(std::string_view const name,
               Background bg,
               Draggable ctrl,
               planet::audio::channel &c)
        : channel{c},
          range{{name,
                 std::move(bg),
                 {name, std::move(ctrl)},
                 {channel.gain().dB, -57, 9}}} {}


        planet::audio::channel &channel;
        planet::ui::box<range_type> range;


        /// ### Forwarding

        void add_to(planet::ui::baseplate &bp) { range.content.add_to(bp); }
        void draw() { range.draw(); }
        constrained_type
                reflow(reflow_parameters const &p, constrained_type const &c) {
            range.content.updater = this;
            return range.reflow(p, c);
        }
        void
                move_to(reflow_parameters const &p,
                        planet::affine::rectangle2d const &r) {
            range.move_to(p, r);
        }


      private:
        void update(float const dB) override {
            channel.update(planet::audio::dB_gain{dB});
        }
    };


}
