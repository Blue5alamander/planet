#include <planet/asset_manager.hpp>
#include <planet/audio.hpp>

#include <felspar/memory/hexdump.hpp>

extern "C" {
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
}

#include <iostream>
#include <map>


using namespace std::literals;
using namespace planet::audio::literals;
int main(int const argc, char const *const argv[]) {
    std::cout << ".ogg file player\n";

    if (argc == 2) {
        std::cout << "Loading " << argv[1] << '\n';
        planet::audio::ogg decoder{planet::file_loader::file_data(argv[1])};

        snd_pcm_t *pcm = nullptr;
        snd_pcm_hw_params_t *hw = nullptr;

        snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
        snd_pcm_hw_params_malloc(&hw);
        snd_pcm_hw_params_any(pcm, hw);
        snd_pcm_hw_params_set_access(pcm, hw, SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(pcm, hw, SND_PCM_FORMAT_FLOAT);
        snd_pcm_hw_params_set_channels(pcm, hw, 2);
        snd_pcm_hw_params_set_rate(
                pcm, hw, planet::audio::sample_clock::period::den, 0);
        snd_pcm_hw_params(pcm, hw);

        for (auto block : planet::audio::gain(-6_dB, decoder.stereo())) {
            snd_pcm_writei(pcm, block.data(), block.samples());
        }

        snd_pcm_hw_params_free(hw);
        snd_pcm_drain(pcm);
        snd_pcm_close(pcm);

        return 0;
    } else {
        std::cerr << "Usage\n\t" << argv[0] << " music.ogg\n";
        return 1;
    }
}
