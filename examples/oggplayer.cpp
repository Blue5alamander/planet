#include <planet/asset_manager.hpp>

#include <felspar/memory/hexdump.hpp>

#include <ogg/ogg.h>

#include <iostream>
#include <map>


int main(int const argc, char const *const argv[]) {
    std::cout << ".ogg file player\n";
    if (argc == 2) {
        std::cout << "Loading " << argv[1] << '\n';
        auto const ogg = planet::file_loader::file_data(argv[1]);

        ogg_sync_state oss{};
        ogg_sync_init(&oss); // Always works

        std::byte *const buffer = reinterpret_cast<std::byte *>(
                ogg_sync_buffer(&oss, ogg.size()));
        std::copy(ogg.begin(), ogg.end(), buffer);
        ogg_sync_wrote(&oss, ogg.size());

        std::map<int, ogg_stream_state> streams;
        ogg_page op;
        while (ogg_sync_pageout(&oss, &op) == 1) {
            std::cout << "header: "
                      << felspar::memory::hexdump(std::span{
                                 op.header,
                                 static_cast<std::size_t>(op.header_len)});

            int serialno = ogg_page_serialno(&op);
            if (not streams.contains(serialno)) {
                ogg_stream_init(&streams[serialno], serialno);
            }
            ogg_stream_pagein(&streams[serialno], &op); // Check for failure

            ogg_packet packet = {};
            int pread = ogg_stream_packetout(&streams[serialno], &packet);

            std::cout << "stream: " << serialno
                      << " ogg_stream_packetout: " << pread << ' '
                      << " number: " << packet.packetno << ' '
                      << felspar::memory::hexdump(std::span{
                                 packet.packet,
                                 static_cast<std::size_t>(packet.bytes)});
        }

        return 0;
    } else {
        std::cerr << "Usage\n\t" << argv[0] << " music.ogg\n";
        return 1;
    }
}
