#pragma once


#include <felspar/memory/shared_buffer.hpp>


namespace planet::audio {


    template<typename Clock, std::size_t Channels>
    class buffer_view;
    template<typename Clock, std::size_t Channels>
    class buffer_storage;


    /// ## Audio buffer
    template<typename Clock, std::size_t Channels>
    class buffer_storage
    /**
     * The `Clock` is the audio clock that is used to determine the sample
     * frequency. For example, `planet::audio::sample_clock` will give a
     * standard 48KHz sample frequency.
     *
     * The buffer is "interleaved", meaning that samples for each channel are
     * next to each other in memory.
     */
    {
        felspar::memory::shared_buffer<float> storage;

      public:
        using buffer_type = felspar::memory::shared_buffer<float>;
        using clock_type = Clock;
        static constexpr std::size_t samples_per_second = Clock::period::den;
        static constexpr std::size_t channels = Channels;


        buffer_storage(buffer_type bt) : storage{std::move(bt)} {}


        float const *data() const { return storage.data(); }
        std::size_t samples() const { return storage.size() / channels; }
        auto duration() const { return clock_type{samples()}; }


        /// ### Access channel sample data
        auto operator[](std::size_t index) const {
            return std::span<float const, channels>{
                    storage.data() + index * channels, channels};
        }


        /// ### Copy first part of the buffer
        buffer_storage first(std::size_t samples) {
            return storage.first(samples * channels);
        }
    };


    /// ## A view onto a buffer
    /// TODO Complete implementation?
    template<typename Clock, std::size_t Channels>
    class buffer_view {
      public:
        using clock_type = Clock;
        static constexpr std::size_t channels = Channels;
    };


}
