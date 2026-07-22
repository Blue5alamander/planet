#pragma once


#include <planet/audio/channel.hpp>
#include <planet/ui/box.hpp>
#include <planet/ui/range.hpp>

#include <type_traits>
#include <variant>


namespace planet::widget::volume {


    /// ## The default gain range a volume slider spans, in decibels
    /**
     * `default_min_dB` is full attenuation and `default_max_dB` a little boost
     * above unity gain (0 dB). The handle travels linearly across this range,
     * so a caller decorating the track (for instance marking 0 dB) can turn a
     * gain into a normalised handle position with these. A single slider can
     * narrow or widen the range through its `parameters`.
     */
    float constexpr default_min_dB = -57.0f;
    float constexpr default_max_dB = 9.0f;


    /// ## Volume slider construction parameters
    /**
     * Gathered into one struct so `slider` has a single constructor and call
     * sites use designated initialisers, naming only the fields they care
     * about. `background`, `cold_spot` and `channel` are required.
     * `min_dB`/`max_dB` default to the range the whole widget shares but can be
     * narrowed or widened per slider.
     *
     * Whether the handle has a distinct hover appearance is chosen by the
     * `HotSpot` type. The default is a reference that aliases the cold spot, so
     * the handle looks the same hovered or not and `hot_spot` holds nothing.
     * Instantiate with a `HotSpot` value type to opt in, and then `hot_spot`
     * carries the value drawn while the pointer is over the handle.
     *
     * `mute_at_minimum` (default `true`) makes the handle mute to true silence
     * once it reaches the floor of its range. Clear it for a channel that
     * should stay audible at the bottom, so the floor is just its quietest
     * gain rather than an off position.
     */
    template<typename Background, typename ColdSpot, typename HotSpot = ColdSpot &>
    struct parameters {
        /// #### Storage for the optional hover spot
        /**
         * Nothing (an empty `std::monostate`) when `HotSpot` is the default
         * aliasing reference, so a non-hover call site simply omits it; the
         * `HotSpot` value itself when a hover appearance is opted into.
         */
        using hot_spot_type = std::conditional_t<
                std::is_reference_v<HotSpot>,
                std::monostate,
                HotSpot>;

        std::string_view name = "planet::widget::volume::slider";
        Background background;
        ColdSpot cold_spot;
        [[no_unique_address]] hot_spot_type hot_spot = {};
        planet::audio::channel &channel;
        float min_dB = default_min_dB;
        float max_dB = default_max_dB;
        bool mute_at_minimum = true;
    };


    /// ## Volume slider
    template<typename Background, typename ColdSpot, typename HotSpot = ColdSpot &>
    struct slider final : public planet::ui::range_updater {
        using range_type = planet::ui::
                range<Background, planet::ui::draggable<ColdSpot, HotSpot>>;
        using parameters = volume::parameters<Background, ColdSpot, HotSpot>;
        using constrained_type = typename range_type::constrained_type;
        using reflow_parameters = typename range_type::reflow_parameters;


        explicit slider(parameters p)
        : channel{p.channel},
          range{make_box(std::move(p))},
          min_dB{p.min_dB},
          mute_at_minimum{p.mute_at_minimum} {}


        planet::audio::channel &channel;
        planet::ui::box<range_type> range;


        /// ### Mute handling
        /**
         * At or below the floor of its range (`min_dB`) the slider mutes to
         * true silence; anywhere above, the channel simply takes the gain the
         * handle rests at. `default_min_dB` is only about -57 dB — quiet but
         * not silent — so without this the slider could never fully turn the
         * channel off.
         *
         * When `mute_at_minimum` is `false` the floor no longer mutes; the
         * handle takes its gain everywhere, so `min_dB` is just the quietest
         * setting.
         */

        /// ##### The gain floor; the handle mutes to silence at or below it
        float min_dB;
        /// ##### Whether reaching the floor mutes the channel to silence
        bool mute_at_minimum;


        /// ### Forwarding

        void add_to(planet::ui::baseplate &bp) { range.content.add_to(bp); }
        void draw() { range.draw(); }
        constrained_type
                reflow(reflow_parameters const &p, constrained_type const &c) {
            range.content.updater = this;
            return range.reflow(p, c);
        }
        planet::affine::rectangle2d
                move_to(reflow_parameters const &p,
                        planet::affine::rectangle2d const &r) {
            return range.move_to(p, r);
        }
        constrained_type const &constraints() const {
            return range.constraints();
        }


      private:
        /// ### Build the range box from the parameters
        static planet::ui::box<range_type> make_box(parameters p)
        /**
         * The handle is hoverable exactly when a `hot_spot` was supplied, which
         * selects the two- or one-argument `draggable` constructor.
         */
        {
            planet::ui::constrained1d<float> const position{
                    p.channel.gain().dB, p.min_dB, p.max_dB};
            if constexpr (std::is_reference_v<HotSpot>) {
                return {
                        {p.name,
                         std::move(p.background),
                         {p.name, std::move(p.cold_spot)},
                         position}};
            } else {
                return {
                        {p.name,
                         std::move(p.background),
                         {p.name, std::move(p.cold_spot), std::move(p.hot_spot)},
                         position}};
            }
        }
        void update(float const dB) override {
            if (mute_at_minimum and dB <= min_dB) {
                channel.update(planet::audio::dB_gain::silence());
            } else {
                channel.update(planet::audio::dB_gain{dB});
            }
        }
    };


    template<typename Background, typename ColdSpot, typename HotSpot>
    slider(parameters<Background, ColdSpot, HotSpot>)
            -> slider<Background, ColdSpot, HotSpot>;


}
