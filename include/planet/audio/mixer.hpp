#pragma once


#include <thread>


namespace planet::audio {


    /// A mixer is a thread that can be given audio inputs and produces audio
    /// output
    class mixer final {
        std::thread thread;
        void mix_thread();

      public:
        mixer();
        ~mixer();
    };


}
