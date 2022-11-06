#pragma once


#include <planet/audio/buffer.hpp>

#include <felspar/coro/generator.hpp>


namespace planet::audio {


    /// Stamps out zeros forever
    felspar::coro::generator<std::span<float>> silence();


    /// A basic single frequency oscillator. The parameter is the number of
    /// turns between samples
    felspar::coro::generator<std::span<float>> oscillator(float turns);


    /// Turn raw samples into a mono buffer
    template<typename Clock>
    inline felspar::coro::generator<buffer_storage<Clock, 1>> monobuffer(
            felspar::coro::generator<std::span<float>> sample_generator) {
        for (auto samples : sample_generator) {
            buffer_storage<Clock, 1> buffer{samples.size()};
            for (std::size_t index{}; auto const sample : samples) {
                buffer[index][0] = sample;
            }
            co_yield std::move(buffer);
        }
    }


    /// Converts a mono buffer to a stereo buffer
    template<typename Clock>
    inline felspar::coro::generator<buffer_storage<Clock, 2>> stereobuffer(
            felspar::coro::generator<buffer_storage<Clock, 1>> mono) {
        for (auto block : mono) {
            buffer_storage<Clock, 2> buffer{block.samples()};
            for (std::size_t index{}; index < block.samples(); ++index) {
                buffer[index][0] = buffer[index][1] = block[index][0];
            }
            co_yield std::move(buffer);
        }
    }


}
