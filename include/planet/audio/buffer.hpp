#pragma once


#include <span>
#include <vector>


namespace planet::audio {


    template<typename Clock, std::size_t Channels>
    class buffer_view;
    template<typename Clock, std::size_t Channels>
    class buffer_storage;


    /// Audio buffer
    /**
     * The `Clock` is the audio clock that is used to determine the sample
     * frequency. For example, `planet::audio::sample_clock` will give a
     * standard 48KHz sample frequency.
     */
    template<typename Clock, std::size_t Channels>
    class buffer_storage {
        std::vector<float> storage;

      public:
        using clock_type = Clock;
        static constexpr std::size_t samples_per_second = Clock::period::den;
        static constexpr std::size_t channels = Channels;

        buffer_storage(std::size_t const sample_count)
        : storage(sample_count * channels) {}

        float const *data() const { return storage.data(); }
        std::size_t samples() const { return storage.size() / channels; }

        auto operator[](std::size_t index) {
            return std::span<float, channels>{
                    storage.data() + index * channels, channels};
        }
    };


    /// A view onto a buffer
    template<typename Clock, std::size_t Channels>
    class buffer_view {
      public:
        using clock_type = Clock;
        static constexpr std::size_t channels = Channels;
    };


}
