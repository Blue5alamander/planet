#include <planet/audio/files.hpp>


planet::audio::wav::wav(std::span<std::byte> filedata)
: samples{[&]() -> buffer_storage<sample_clock, 2> {
      auto const header = std::span<std::uint8_t>{
              reinterpret_cast<std::uint8_t *>(filedata.data()), 88};

      std::size_t const file_size = header[4] + (header[5] << 8)
              + (header[6] << 16) + (header[7] << 24);
      std::size_t const data_size = header[84] + (header[85] << 8)
              + (header[86] << 16) + (header[87] << 24);

      std::span<float> samples(
              reinterpret_cast<float *>(filedata.data() + 88),
              data_size / sizeof(float));

      buffer_storage<sample_clock, 2> audio{samples.size() / 2};
      for (std::size_t index{}; index < audio.samples(); ++index) {
          audio[index][0] = samples[index * 2];
          audio[index][1] = samples[index * 2 + 1];
      }
      return audio;
  }()} {}


auto planet::audio::wav::output()
        -> felspar::coro::generator<buffer_storage<sample_clock, 2>> {
    for (std::size_t marker{}; marker < samples.samples(); ++marker) {
        buffer_storage<sample_clock, 2> buffer{std::min(
                samples.samples() - marker,
                std::size_t(default_buffer_duration.count()))};
        for (std::size_t index{}; index < buffer.samples(); ++index, ++marker) {
            buffer[index][0] = samples[marker][0];
            buffer[index][1] = samples[marker][1];
        }
        co_yield std::move(buffer);
    }
}
