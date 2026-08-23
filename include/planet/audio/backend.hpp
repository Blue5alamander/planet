#pragma once


#include <planet/audio/clocks.hpp>


namespace planet::audio {


    /// ## Audio output backend
    /**
     * The interface every audio-output backend implements. The implementation
     * of the backend sub-classes this and the configuration user knows which
     * backend to create.
     */
    class backend {
      public:
        virtual ~backend();


        /// ### Playback head
        virtual std::atomic<sample_clock> const &
                playback_clock() const noexcept = 0;
    };


}
