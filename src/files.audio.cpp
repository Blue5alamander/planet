#include <planet/audio/files.hpp>

#include <felspar/memory/accumulation_buffer.hpp>

#include <vorbis/codec.h>


/// ## `planet::audio::ogg::impl`


struct planet::audio::ogg::impl {
    impl(std::span<std::byte>, felspar::source_location const &);
    ~impl();

    vorbis_info vi;
    vorbis_comment vc;
    felspar::source_location loc;

    felspar::coro::generator<
            planet::audio::buffer_storage<planet::audio::sample_clock, 2>>
            stereo();

  private:
    std::span<std::byte> ogg;
    felspar::coro::generator<ogg_packet> packets;
    felspar::coro::generator<ogg_packet> vorbis_packets();
};


planet::audio::ogg::impl::impl(
        std::span<std::byte> o, felspar::source_location const &l)
: loc{l}, ogg{o}, packets{vorbis_packets()} {
    vorbis_info_init(&vi);
    vorbis_comment_init(&vc);
    for (std::size_t expected{3}; expected; --expected) {
        auto vip = packets.next();
        if (vorbis_synthesis_headerin(&vi, &vc, &*vip) < 0) {
            throw felspar::stdexcept::runtime_error{
                    "Not Vorbis audio data", loc};
        }
    }
}


planet::audio::ogg::impl::~impl() {
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
}


felspar::coro::generator<ogg_packet> planet::audio::ogg::impl::vorbis_packets() {
    struct ogg_resources {
        ogg_resources() {
            ogg_sync_init(&sync); // Always works
        }
        ~ogg_resources() {
            if (stream) { ogg_stream_clear(&*stream); }
            ogg_sync_clear(&sync);
        }
        ogg_sync_state sync;
        std::optional<ogg_stream_state> stream;
    };
    ogg_resources oggr;

    ogg_page op;
    if (ogg_sync_pageout(&oggr.sync, &op) != 0) {
        throw felspar::stdexcept::runtime_error{
                "ogg_sync_pageout not requesting data", loc};
    }

    std::byte *const buffer = reinterpret_cast<std::byte *>(
            ogg_sync_buffer(&oggr.sync, ogg.size()));
    std::copy(ogg.begin(), ogg.end(), buffer);
    if (ogg_sync_wrote(&oggr.sync, ogg.size()) == -1) {
        throw felspar::stdexcept::runtime_error{"ogg_sync_wrote failed", loc};
    }

    std::optional<int> streamid;
    while (ogg_sync_pageout(&oggr.sync, &op) == 1) {
        int serialno = ogg_page_serialno(&op);
        if (not streamid) {
            streamid = serialno;
            oggr.stream = ogg_stream_state{};
            ogg_stream_init(&*oggr.stream, serialno);
        }
        if (serialno == streamid) {
            if (ogg_stream_pagein(&*oggr.stream, &op) == -1) {
                throw felspar::stdexcept::runtime_error{
                        "ogg_stream_pagein failed", loc};
            }

            ogg_packet packet;
            while (ogg_stream_packetout(&*oggr.stream, &packet) == 1) {
                co_yield packet;
            }
        }
    }
}


felspar::coro::generator<
        planet::audio::buffer_storage<planet::audio::sample_clock, 2>>
        planet::audio::ogg::impl::stereo() {
    if (vi.channels != 2) {
        throw felspar::stdexcept::runtime_error{
                "This ogg file is not stereo", loc};
    } else if (vi.rate != planet::audio::sample_clock::period::den) {
        throw felspar::stdexcept::runtime_error{
                "The sample rate is wrong. Should be 48kHz and is "
                        + std::to_string(vi.rate) + "Hz",
                loc};
    }
    struct vorbis_resources {
        vorbis_resources(impl *i) {
            vorbis_synthesis_init(&d, &i->vi);
            vorbis_block_init(&d, &b);
        }
        ~vorbis_resources() {
            vorbis_block_clear(&b);
            vorbis_dsp_clear(&d);
        }
        vorbis_dsp_state d;
        vorbis_block b;
    };
    vorbis_resources v{this};

    felspar::memory::accumulation_buffer<float> output{
            default_buffer_samples * 50};

    for (auto &&packet : packets) {
        if (vorbis_synthesis(&v.b, &packet) == 0) {
            vorbis_synthesis_blockin(&v.d, &v.b);
        } else {
            throw felspar::stdexcept::runtime_error{
                    "vorbis_synthesis failed", loc};
        }
        float **pcm = nullptr;
        while (int isamples = vorbis_synthesis_pcmout(&v.d, &pcm)) {
            auto const samples = static_cast<std::size_t>(isamples);
            output.ensure_length(samples * stereo_buffer::channels);
            for (std::size_t sample{}; sample < samples; ++sample) {
                output.at(sample * stereo_buffer::channels + 0) =
                        pcm[0][sample];
                output.at(sample * stereo_buffer::channels + 1) =
                        pcm[1][sample];
            }
            vorbis_synthesis_read(&v.d, samples);
            co_yield output.first(samples * stereo_buffer::channels);
        }
    }
}


/// ## `planet::audio::ogg`


planet::audio::ogg::ogg(
        std::vector<std::byte> o, felspar::source_location const &l)
: filedata{std::move(o)}, loc{l} {}


felspar::coro::generator<
        planet::audio::buffer_storage<planet::audio::sample_clock, 2>>
        planet::audio::ogg::stereo() {
    impl decoder{filedata, loc};
    for (auto s = decoder.stereo(); auto p = s.next();) {
        co_yield std::move(*p);
    }
}
