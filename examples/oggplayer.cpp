#include <planet/asset_manager.hpp>
#include <planet/audio.hpp>

#include <felspar/memory/hexdump.hpp>

extern "C" {
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
}
#include <vorbis/codec.h>

#include <iostream>
#include <map>


using namespace std::literals;
using namespace planet::audio::literals;


namespace {
    struct decoder {
        vorbis_info vi;
        vorbis_comment vc;
        ogg_sync_state sync;
        ogg_stream_state stream;

        felspar::coro::generator<ogg_packet>
                vorbis_packets(std::vector<std::byte> ogg) {
            ogg_sync_init(&sync); // Always works
            vorbis_info_init(&vi);
            vorbis_comment_init(&vc);

            ogg_page op;
            if (ogg_sync_pageout(&sync, &op) != 0) {
                throw std::runtime_error{
                        "ogg_sync_pageout not requesting data"};
            }

            std::byte *const buffer = reinterpret_cast<std::byte *>(
                    ogg_sync_buffer(&sync, ogg.size()));
            std::copy(ogg.begin(), ogg.end(), buffer);
            if (ogg_sync_wrote(&sync, ogg.size()) == -1) {
                throw std::runtime_error{"ogg_sync_wrote failed"};
            }

            std::optional<int> streamid;
            while (ogg_sync_pageout(&sync, &op) == 1) {
                int serialno = ogg_page_serialno(&op);
                if (not streamid) {
                    streamid = serialno;
                    ogg_stream_init(&stream, serialno);
                }
                if (serialno == streamid) {
                    if (ogg_stream_pagein(&stream, &op) == -1) {
                        throw std::runtime_error{"ogg_stream_pagein failed"};
                    }

                    ogg_packet packet;
                    while (ogg_stream_packetout(&stream, &packet) == 1) {
                        co_yield packet;
                    }
                }
            }
        }

        felspar::coro::generator<
                planet::audio::buffer_storage<planet::audio::sample_clock, 2>>
                stereo(std::vector<std::byte> ogg) {
            auto packets = vorbis_packets(std::move(ogg));
            for (std::size_t expected{3}; expected; --expected) {
                auto vip = packets.next();
                if (vorbis_synthesis_headerin(&vi, &vc, &*vip) < 0) {
                    throw std::runtime_error{"Not Vorbis audio data"};
                }
            }

            vorbis_dsp_state vd;
            vorbis_synthesis_init(&vd, &vi);
            vorbis_block vb;
            vorbis_block_init(&vd, &vb);

            for (auto &&packet : packets) {
                if (vorbis_synthesis(&vb, &packet) == 0) {
                    vorbis_synthesis_blockin(&vd, &vb);
                } else {
                    throw std::runtime_error{"vorbis_synthesis failed"};
                }
                float **pcm = nullptr;
                while (int samples = vorbis_synthesis_pcmout(&vd, &pcm)) {
                    planet::audio::buffer_storage<planet::audio::sample_clock, 2>
                            buffer{static_cast<std::size_t>(samples)};
                    for (std::size_t sample{}; sample < buffer.samples();
                         ++sample) {
                        buffer[sample][0] = pcm[0][sample];
                        buffer[sample][1] = pcm[1][sample];
                    }
                    vorbis_synthesis_read(&vd, samples);
                    co_yield std::move(buffer);
                }
            }

            vorbis_block_clear(&vb);
            vorbis_dsp_clear(&vd);
            ogg_stream_clear(&stream);
            vorbis_comment_clear(&vc);
            vorbis_info_clear(&vi);
            ogg_sync_clear(&sync);

            co_return;
        }
    };
}


int main(int const argc, char const *const argv[]) {
    std::cout << ".ogg file player\n";

    if (argc == 2) {
        std::cout << "Loading " << argv[1] << '\n';

        snd_pcm_t *pcm = nullptr;
        snd_pcm_hw_params_t *hw = nullptr;

        snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
        snd_pcm_hw_params_malloc(&hw);
        snd_pcm_hw_params_any(pcm, hw);
        snd_pcm_hw_params_set_access(pcm, hw, SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(pcm, hw, SND_PCM_FORMAT_FLOAT);
        snd_pcm_hw_params_set_channels(pcm, hw, 2);
        snd_pcm_hw_params_set_rate(pcm, hw, 44100, 0);
        snd_pcm_hw_params(pcm, hw);

        ::decoder decoder;
        for (auto block : planet::audio::gain(
                     -6_dB,
                     decoder.stereo(planet::file_loader::file_data(argv[1])))) {
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
