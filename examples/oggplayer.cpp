#include <planet/asset_manager.hpp>

#include <felspar/memory/hexdump.hpp>

#include <vorbis/codec.h>

#include <iostream>
#include <map>


namespace {
    struct stream_info {
        vorbis_info vi;
        vorbis_comment vc;
        ogg_stream_state stream;
        ogg_sync_state sync;
    };
    struct vorbis_packet {
        int serialno;
        stream_info &info;
        ogg_packet &packet;
    };

    felspar::coro::generator<vorbis_packet>
            vorbis_packets(std::vector<std::byte> ogg) {
        ogg_sync_state sync{};
        ogg_sync_init(&sync); // Always works

        std::byte *const buffer = reinterpret_cast<std::byte *>(
                ogg_sync_buffer(&sync, ogg.size()));
        std::copy(ogg.begin(), ogg.end(), buffer);
        ogg_sync_wrote(&sync, ogg.size());

        std::map<int, stream_info> streams;

        ogg_page op;
        while (ogg_sync_pageout(&sync, &op) == 1) {
            int serialno = ogg_page_serialno(&op);
            if (not streams.contains(serialno)) {
                ogg_stream_init(&streams[serialno].stream, serialno);
                vorbis_info_init(&streams[serialno].vi);
                vorbis_comment_init(&streams[serialno].vc);
                streams[serialno].sync = sync;
            }
            ogg_stream_pagein(
                    &streams[serialno].stream, &op); // Check for failure

            ogg_packet packet = {};
            ogg_stream_packetout(&streams[serialno].stream, &packet);

            co_yield {serialno, streams[serialno], packet};
        }
    }
}


int main(int const argc, char const *const argv[]) {
    std::cout << ".ogg file player\n";
    if (argc == 2) {
        std::cout << "Loading " << argv[1] << '\n';

        auto packets = vorbis_packets(planet::file_loader::file_data(argv[1]));

        stream_info info;

        for (std::size_t expected{3}; expected; --expected) {
            auto vip = packets.next();
            if (vorbis_synthesis_headerin(
                        &vip->info.vi, &vip->info.vc, &vip->packet)
                < 0) {
                throw std::runtime_error{"Not Vorbis audio data"};
            } else {
                info = vip->info;
            }
        }

        std::cout << "Bitstream is " << info.vi.channels << " channels at "
                  << info.vi.rate << "Hz\n";

        vorbis_dsp_state vd;
        vorbis_synthesis_init(&vd, &info.vi);
        vorbis_block vb;
        vorbis_block_init(&vd, &vb);

        for (auto &&vp : packets) {
            if (vorbis_synthesis(&vb, &vp.packet) == 0) {
                vorbis_synthesis_blockin(&vd, &vb);
            } else {
                throw std::runtime_error{"vorbis_synthesis failed"};
            }
            float **pcm = nullptr;
            while (int samples = vorbis_synthesis_pcmout(&vd, &pcm) > 0) {
                std::cout << "pcm: "
                          << felspar::memory::hexdump(std::span{
                                     reinterpret_cast<std::byte *>(pcm[0]),
                                     static_cast<std::size_t>(
                                             samples * sizeof(float))});
                vorbis_synthesis_read(&vd, samples);
            }
        }

        vorbis_block_clear(&vb);
        vorbis_dsp_clear(&vd);
        // ogg_stream_clear(&info.stream);
        // vorbis_comment_clear(&info.vc);
        // vorbis_info_clear(&info.vi);
        // ogg_sync_clear(&info.sync);

        return 0;
    } else {
        std::cerr << "Usage\n\t" << argv[0] << " music.ogg\n";
        return 1;
    }
}
