#pragma once


#include <planet/audio/gain.hpp>
#include <planet/ui/constrained.hpp>


namespace planet::audio {


    /// ## Audio channel
    /**
     * Abstracts out the notion of a particular audio output with its own gain.
     */
    class channel {
        dB_gain &gain;
        atomic_linear_gain linear;


      public:
        channel(dB_gain &g) : gain{g} {}


        template<typename Clock, std::size_t Channels>
        felspar::coro::generator<buffer_storage<Clock, Channels>> attenuate(
                felspar::coro::generator<buffer_storage<Clock, Channels>> s) {
            return audio::gain(linear, std::move(s));
        }


        void update(ui::constrained1d<float> c) {
            gain.dB = c.value();
            linear.store(static_cast<linear_gain>(gain));
        }
    };


}
