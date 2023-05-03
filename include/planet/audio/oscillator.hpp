#pragma once


#include <planet/audio/buffer.hpp>

#include <felspar/coro/generator.hpp>
#include <felspar/memory/accumulation_buffer.hpp>


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
        felspar::memory::accumulation_buffer<float> output{
                default_buffer_samples * 50};
        for (auto samples : sample_generator) {
            output.ensure_length(samples.size());
            for (std::size_t index{}; auto const sample : samples) {
                output[index++] = sample;
            }
            co_yield output.first(samples.size());
        }
    }


}
