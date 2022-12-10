#pragma once


#include <planet/audio/buffer.hpp>
#include <planet/audio/clocks.hpp>

#include <felspar/coro/generator.hpp>

#include <thread>


namespace planet::audio {


    /// Can be given arbitrary input streams and produces output of the same format
    template<typename Buffer>
    class mixer final {
      public:
        using io_type = felspar::coro::generator<Buffer>;

        mixer() {}
        mixer(mixer const &) = delete;
        mixer(mixer &&) = delete;
        mixer &operator=(mixer const &) = delete;
        mixer &operator=(mixer &&) = delete;

        void add_track(io_type track) {
            generators.push_back(std::move(track));
        }

        io_type output() {
            while (true) {
                Buffer output{default_buffer_samples};
                std::erase_if(generators, [&output](auto &g) {
                    auto next = g.next();
                    if (next) {
                        for (std::size_t index{}; index < next->samples();
                             ++index) {
                            for (std::size_t ch{}; ch < Buffer::channels;
                                 ++ch) {
                                output[index][ch] = (*next)[index][ch];
                            }
                        }
                        return false;
                    } else {
                        return true;
                    }
                });
                co_yield std::move(output);
            }
        }

      private:
        std::vector<io_type> generators;
    };


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
