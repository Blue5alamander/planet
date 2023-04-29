#include <planet/audio/files.hpp>

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
    ogg_sync_state sync;
    ogg_stream_state stream;
    ogg_sync_init(&sync); // Always works

    ogg_page op;
    if (ogg_sync_pageout(&sync, &op) != 0) {
        throw felspar::stdexcept::runtime_error{
                "ogg_sync_pageout not requesting data", loc};
    }

    std::byte *const buffer =
            reinterpret_cast<std::byte *>(ogg_sync_buffer(&sync, ogg.size()));
    std::copy(ogg.begin(), ogg.end(), buffer);
    if (ogg_sync_wrote(&sync, ogg.size()) == -1) {
        throw felspar::stdexcept::runtime_error{"ogg_sync_wrote failed", loc};
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
                throw felspar::stdexcept::runtime_error{
                        "ogg_stream_pagein failed", loc};
            }

            ogg_packet packet;
            while (ogg_stream_packetout(&stream, &packet) == 1) {
                co_yield packet;
            }
        }
    }
    /// TODO Make exception safe
    ogg_stream_clear(&stream);
    ogg_sync_clear(&sync);
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

    vorbis_dsp_state vd;
    vorbis_synthesis_init(&vd, &vi);
    vorbis_block vb;
    vorbis_block_init(&vd, &vb);

    for (auto &&packet : packets) {
        if (vorbis_synthesis(&vb, &packet) == 0) {
            vorbis_synthesis_blockin(&vd, &vb);
        } else {
            throw felspar::stdexcept::runtime_error{
                    "vorbis_synthesis failed", loc};
        }
        float **pcm = nullptr;
        while (int samples = vorbis_synthesis_pcmout(&vd, &pcm)) {
            planet::audio::buffer_storage<planet::audio::sample_clock, 2> buffer{
                    static_cast<std::size_t>(samples)};
            for (std::size_t sample{}; sample < buffer.samples(); ++sample) {
                buffer[sample][0] = pcm[0][sample];
                buffer[sample][1] = pcm[1][sample];
            }
            vorbis_synthesis_read(&vd, samples);
            co_yield std::move(buffer);
        }
    }

    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);

    co_return;
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


/// ## `planet::audio::wav`


planet::audio::wav::wav(std::span<std::byte const> filedata)
: samples{[&]() -> buffer_storage<sample_clock, 2> {
      auto const header = std::span<std::uint8_t const>{
              reinterpret_cast<std::uint8_t const *>(filedata.data()), 88};

      [[maybe_unused]] std::size_t const file_size = header[4]
              + (header[5] << 8) + (header[6] << 16) + (header[7] << 24);
      std::size_t const data_size = header[84] + (header[85] << 8)
              + (header[86] << 16) + (header[87] << 24);

      std::span<float const> samples(
              reinterpret_cast<float const *>(filedata.data() + 88),
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
