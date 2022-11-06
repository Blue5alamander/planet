#include <planet/audio.hpp>

extern "C" {
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
}


int main() {
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

    auto generator = planet::audio::stereobuffer(
            planet::audio::monobuffer<planet::audio::sample_clock>(
                    planet::audio::oscillator(
                            440.0f / planet::audio::sample_clock::period::den)));

    for (auto block : generator) {
        snd_pcm_writei(pcm, block.data(), block.samples());
    }

    snd_pcm_hw_params_free(hw);
    snd_pcm_drain(pcm);
    snd_pcm_close(pcm);
}
