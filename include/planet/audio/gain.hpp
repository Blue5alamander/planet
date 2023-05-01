#pragma once


#include <planet/audio/buffer.hpp>

#include <felspar/coro/generator.hpp>

#include <atomic>


namespace planet::audio {


    /// ## Linear gain
    class linear_gain {
        friend class atomic_linear_gain;
        float multiplier = {1};

      public:
        linear_gain() {}
        explicit linear_gain(float);

        float operator*(float const v) const noexcept { return multiplier * v; }
    };


    /// ## Linear gain using an atomic multiplier
    class atomic_linear_gain {
        std::atomic<float> multiplier = {1};

      public:
        atomic_linear_gain() {}
        explicit atomic_linear_gain(float);

        void set(linear_gain);

        float operator*(float const v) const noexcept {
            return multiplier.load(std::memory_order_relaxed) * v;
        }
    };


    /// ## Gain in +/- dB
    class dB_gain {
        float dB = {};

      public:
        dB_gain() {}
        explicit dB_gain(float);

        explicit operator linear_gain() const noexcept;

        dB_gain operator-() const noexcept { return dB_gain{-dB}; }
    };


    /// ## Apply a gain level to the audio
    template<typename Clock, std::size_t Channels, typename LinearGain>
    inline felspar::coro::generator<buffer_storage<Clock, Channels>> gain(
            LinearGain const gain,
            felspar::coro::generator<buffer_storage<Clock, Channels>> signal) {
        for (auto block : signal) {
            buffer_storage<Clock, Channels> buffer{block.samples()};
            for (std::size_t index{}; index < block.samples(); ++index) {
                for (std::size_t channel{}; channel < Channels; ++channel) {
                    buffer[index][channel] = gain * block[index][channel];
                }
            }
            co_yield std::move(buffer);
        }
    }
    template<typename Clock, std::size_t Channels>
    inline felspar::coro::generator<buffer_storage<Clock, Channels>> gain(
            dB_gain const dB,
            felspar::coro::generator<buffer_storage<Clock, Channels>> signal) {
        return gain(linear_gain(dB), std::move(signal));
    }


    namespace literals {
        inline auto operator"" _dB(unsigned long long d) {
            return dB_gain{float(d)};
        }
        inline auto operator"" _dB(long double d) { return dB_gain{float(d)}; }
    }


}
