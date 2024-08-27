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
        /**
         * TODO This would have to change from using `std::function` if we want
         * to be able to use the lifetime tracking features of clang for
         * coroutines. This is because `std::function`'s `operator()` is not
         * marked with the correct attribute.
         */
        using start_tune_function = std::function<stereo_generator()>;


        /// ### Playing flag
        /// Returns true if music is currently playing
        bool is_playing() const noexcept;


        /// ### Construction and output

        /// #### Audio output
        /// The generator will be handled to the audio processing thread
        stereo_generator output();


        /// ### Queuing music

        /// #### Stop playing and clear the queue
        felspar::coro::task<void> clear(felspar::io::warden &);

        /// #### Queue this to play
        void enqueue(start_tune_function);


        /// ### Volume control

        /// #### The volume to set on the playback
        /**
         * A value less than or equal to -127dB will be taken as mute and the
         * playback will pause
         */
        void set_volume(dB_gain);


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
        dB_gain master{-9};
        /// The gain value that is used during playback
        atomic_linear_gain master_gain;
    };


}
