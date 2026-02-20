#pragma once


#include <planet/audio/buffer.hpp>
#include <planet/audio/clocks.hpp>
#include <planet/audio/forward.hpp>
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


        float load() const noexcept { return multiplier; }
    };


    /// ## Linear gain using an atomic multiplier
    class atomic_linear_gain {
        std::atomic<float> multiplier = {1};

      public:
        atomic_linear_gain() {}
        explicit atomic_linear_gain(dB_gain gain) noexcept;
        explicit atomic_linear_gain(linear_gain lg) noexcept
        : multiplier{lg.multiplier} {}


        void store(linear_gain);
        float load() const noexcept {
            return multiplier.load(std::memory_order_relaxed);
        }
    };


    /// ## Gain in +/- dB
    struct dB_gain {
        static constexpr std::string_view box{"_p:a:dB"};


        float dB = {};


        constexpr dB_gain() {}
        constexpr explicit dB_gain(float const g) : dB{g} {}


        operator linear_gain() const noexcept;


        constexpr dB_gain operator-() const noexcept { return dB_gain{-dB}; }

        constexpr dB_gain &operator-=(dB_gain const g) noexcept {
            dB -= g.dB;
            return *this;
        }


        friend constexpr bool
                operator==(dB_gain const &, dB_gain const &) noexcept = default;
        friend constexpr auto operator<=>(
                dB_gain const &, dB_gain const &) noexcept = default;
        std::string as_string() const;
    };
    void save(serialise::save_buffer &, dB_gain const &);
    void load(serialise::box &, dB_gain &);


    /// ## Apply a gain level to the audio
    template<typename Clock, std::size_t Channels>
    inline felspar::coro::generator<buffer_storage<Clock, Channels>> gain(
            linear_gain gain,
            felspar::coro::generator<buffer_storage<Clock, Channels>> signal) {
        auto const mul = gain.load();
        felspar::memory::accumulation_buffer<float> output{
                default_buffer_samples * Channels * 25};

        for (auto block : signal) {
            output.ensure_length(block.samples() * Channels);
            for (std::size_t index{}; index < block.samples(); ++index) {
                for (std::size_t channel{}; channel < Channels; ++channel) {
                    output[index * Channels + channel] =
                            mul * block[index][channel];
                }
            }
            co_yield output.first(block.samples() * Channels);
        }
    }
    template<typename Clock, std::size_t Channels>
    inline felspar::coro::generator<buffer_storage<Clock, Channels>> gain(
            atomic_linear_gain const &gain,
            felspar::coro::generator<buffer_storage<Clock, Channels>> signal) {
        felspar::memory::accumulation_buffer<float> output{
                default_buffer_samples * Channels * 25};
        auto old_mul = gain.load();
        for (auto block : signal) {
            output.ensure_length(block.samples() * Channels);
            auto const mul = gain.load();
            float const t_length = block.samples();
            for (std::size_t index{}; index < block.samples(); ++index) {
                for (std::size_t channel{}; channel < Channels; ++channel) {
                    auto const this_mul =
                            std::lerp(old_mul, mul, index / t_length);
                    output[index * Channels + channel] =
                            this_mul * block[index][channel];
                }
            }
            old_mul = mul;
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


    /// ## Fade in/out
    template<typename Clock, std::size_t Channels>
    [[nodiscard]]
    inline auto micro_fade_out(
            buffer_storage<Clock, Channels> &block,
            felspar::memory::accumulation_buffer<float> &output) {
        auto const float_count = block.samples() * Channels;
        output.ensure_length(float_count);
        for (std::size_t sample{}; sample < block.samples(); ++sample) {
            dB_gain const gain{std::lerp(
                    0.0f, -90.0f, sample / static_cast<float>(block.samples()))};
            auto const mul{linear_gain{gain}.load()};
            for (std::size_t channel{}; channel < Channels; ++channel) {
                output[sample * Channels + channel] =
                        block[sample][channel] * mul;
            }
        }
        return output.first(float_count);
    }


    namespace literals {
        inline auto operator""_dB(unsigned long long d) {
            return dB_gain{float(d)};
        }
        inline auto operator""_dB(long double d) { return dB_gain{float(d)}; }
    }


}
