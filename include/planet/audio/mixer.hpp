#pragma once


#include <planet/audio/buffer.hpp>
#include <planet/audio/clocks.hpp>

#include <thread>


namespace planet::audio {


    /// Can be given arbitrary input streams and produces output of the same format
    template<typename Buffer>
    class mixer final {};


    /// mix2pipe controls a thread that can be given audio inputs and produces
    /// audio output. The samples are written to the pipe and can be read from
    /// another thread. The implementation always uses 48KHz stereo buffers
    class mix2pipe final {
        std::thread thread;
        void mix_thread();

      public:
        using mixer_type = mixer<buffer_view<sample_clock, 2>>;

        mix2pipe();
        ~mix2pipe();
    };


}
