#pragma once


#include <planet/audio/channel.hpp>
#include <planet/audio/gain.hpp>
#include <planet/audio/stereo.hpp>

#include <felspar/coro/task.hpp>
#include <felspar/io/warden.hpp>

#include <mutex>


namespace planet::audio {


    /// ## Music queue
    /**
     * The expectation is that the music queue needs to be controlled from
     * separate threads to the one where the music is produced and put into the
     * mixer.
     */
    class music {
      public:
        /// ### Construction and output
        music(channel &c) : master{c} {}

        /// #### Audio output
        stereo_generator output();
        /// The generator will be handled to the audio processing thread


        using start_tune_function = felspar::coro::generator<stereo_generator>;


        /// ### Playing flag
        /// Returns true if music is currently playing
        bool is_playing() const noexcept;


        /// ### Queuing music

        /// #### Stop playing and clear the queue
        felspar::coro::task<void> clear(felspar::io::warden &);

        /// #### Queue this to play
        void enqueue(start_tune_function);


      private:
        std::atomic<bool> clear_flag = false, playing = false;
        std::mutex mtx;
        std::vector<start_tune_function> queue;
        /**
         * Used to auto-fade between tracks and at start up. This value is always
         * manipulated in the same thread that performs the auto generation.
         */
        linear_gain auto_fade;
        /// The user requested gain value
        channel &master;
    };


}
