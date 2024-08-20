#pragma once


#include <planet/audio/buffer.hpp>
#include <planet/audio/clocks.hpp>
#include <planet/serialise/forward.hpp>

#include <felspar/coro/generator.hpp>
#include <felspar/memory/accumulation_buffer.hpp>

#include <atomic>
#include <cmath>


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
        explicit atomic_linear_gain(linear_gain lg) noexcept
        : multiplier{lg.multiplier} {}

        void set(linear_gain);

        float operator*(float const v) const noexcept {
            return multiplier.load(std::memory_order_relaxed) * v;
        }
    };


    /// ## Gain in +/- dB
    struct dB_gain {
        static constexpr std::string_view box{"_p:a:dB"};


        float dB = {};

        dB_gain() {}
        explicit dB_gain(float);

        explicit operator linear_gain() const noexcept {
            if (dB < -200.0f) {
                return linear_gain{};
            } else {
                return linear_gain{std::pow(10.0f, dB / 20.0f)};
            }
        }
        /// TODO This looks really wrong
        explicit operator atomic_linear_gain() const noexcept {
            return atomic_linear_gain{static_cast<linear_gain>(*this)};
        }

        dB_gain operator-() const noexcept { return dB_gain{-dB}; }
    };
    void save(serialise::save_buffer &, dB_gain const &);
    void load(serialise::box &, dB_gain &);


    /// ## Apply a gain level to the audio
    template<typename Clock, std::size_t Channels, typename LinearGain>
    inline felspar::coro::generator<buffer_storage<Clock, Channels>> gain(
            LinearGain const gain,
            felspar::coro::generator<buffer_storage<Clock, Channels>> signal) {
        felspar::memory::accumulation_buffer<float> output{
                default_buffer_samples * Channels * 25};
        for (auto block : signal) {
            output.ensure_length(block.samples() * Channels);
            for (std::size_t index{}; index < block.samples(); ++index) {
                for (std::size_t channel{}; channel < Channels; ++channel) {
                    output[index * Channels + channel] =
                            gain * block[index][channel];
                }
            }
            co_yield output.first(block.samples() * Channels);
        }
    }
    template<typename Clock, std::size_t Channels>
    FELSPAR_CORO_WRAPPER inline felspar::coro::generator<
            buffer_storage<Clock, Channels>>
            gain(dB_gain const dB,
                 felspar::coro::generator<buffer_storage<Clock, Channels>>
                         signal) {
        return gain(linear_gain(dB), std::move(signal));
    }


    namespace literals {
        inline auto operator"" _dB(unsigned long long d) {
            return dB_gain{float(d)};
        }
        inline auto operator"" _dB(long double d) { return dB_gain{float(d)}; }
    }


}
