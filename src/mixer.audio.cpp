#include <planet/audio/mixer.hpp>

#include <felspar/memory/accumulation_buffer.hpp>


auto planet::audio::mixer::output() -> stereo_generator {
    felspar::memory::accumulation_buffer<float> output{
            default_buffer_samples * 50};
    while (true) {
        std::erase_if(generators, [&output](auto &gen) {
            while (gen.samples < default_buffer_samples) {
                auto buffer = gen.audio.next();
                if (buffer) {
                    output.ensure_length(
                            (gen.samples + buffer->samples())
                            * stereo_buffer::channels);
                    for (std::size_t sample{}; sample < buffer->samples();
                         ++sample) {
                        std::size_t const idx = (gen.samples + sample)
                                            * stereo_buffer::channels;
                        auto const src = (*buffer)[sample];
                        output.at(idx + 0) += src[0];
                        output.at(idx + 1) += src[1];
                    }
                    gen.samples += buffer->samples();
                } else {
                    return true;
                }
            }
            gen.samples -= default_buffer_samples;
            return false;
        });
        output.ensure_length(default_buffer_samples * stereo_buffer::channels);
        co_yield output.first(default_buffer_samples * stereo_buffer::channels);
    }
}
