#pragma once


#include <planet/audio/gain.hpp>
#include <planet/serialise/forward.hpp>
#include <planet/ui/constrained.hpp>


namespace planet::audio {


    /// ## Audio channel
    /**
     * Abstracts out the notion of a particular audio output with its own gain.
     */
    class channel {
        dB_gain db_g;
        atomic_linear_gain linear;

        void write_through() noexcept {
            linear.store(static_cast<linear_gain>(db_g));
        }


      public:
        static constexpr std::string_view box{"_p:a:channel"};


        channel(dB_gain const g) : db_g{g} {}


        template<typename Clock, std::size_t Channels>
        felspar::coro::generator<buffer_storage<Clock, Channels>> attenuate(
                felspar::coro::generator<buffer_storage<Clock, Channels>> s) {
            return audio::gain(linear, std::move(s));
        }


        dB_gain const &gain() const noexcept { return db_g; }


        void update(dB_gain const g) noexcept {
            db_g = g;
            write_through();
        }


        friend void save(serialise::save_buffer &, channel const &);
        friend void load(serialise::box &, channel &);
    };
    void save(serialise::save_buffer &, channel const &);
    void load(serialise::box &, channel &);


}
