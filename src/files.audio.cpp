#include <planet/audio/files.hpp>

#include <felspar/exceptions/runtime_error.hpp>
#include <felspar/memory/accumulation_buffer.hpp>
#include <felspar/parse/extract.be.hpp>
#include <felspar/parse/extract.le.hpp>

#include <algorithm>
#include <optional>

#include <FLAC/stream_decoder.h>
#include <vorbis/codec.h>


/// ## `planet::audio::flac::impl`
/**
 * libFLAC pushes decoded audio to a write callback rather than letting us pull
 * it, so we drive the decoder one frame at a time with
 * `FLAC__stream_decoder_process_single` and hand the frame the callback just
 * produced back out of the generator. The whole file is already in memory, so
 * the read callback simply serves bytes from `flacdata`; the seek/tell/length
 * callbacks are left null as the decoder does not need to seek a forward,
 * one-shot decode.
 */


struct planet::audio::flac::impl {
    impl(std::span<std::byte const>, std::source_location const &);
    ~impl();

    std::source_location loc;

    felspar::coro::generator<
            planet::audio::buffer_storage<planet::audio::sample_clock, 2>>
            stereo();

  private:
    std::span<std::byte const> flacdata;
    std::size_t read_position{};

    FLAC__StreamDecoder *decoder{};
    std::optional<FLAC__StreamMetadata_StreamInfo> stream_info;
    char const *error{};

    felspar::memory::accumulation_buffer<float> output{
            default_buffer_samples * 50};
    std::size_t frame_samples{};

    static FLAC__StreamDecoderReadStatus read_callback(
            FLAC__StreamDecoder const *, FLAC__byte[], std::size_t *, void *);
    static FLAC__StreamDecoderWriteStatus write_callback(
            FLAC__StreamDecoder const *,
            FLAC__Frame const *,
            FLAC__int32 const *const[],
            void *);
    static void metadata_callback(
            FLAC__StreamDecoder const *, FLAC__StreamMetadata const *, void *);
    static void error_callback(
            FLAC__StreamDecoder const *, FLAC__StreamDecoderErrorStatus, void *);
};


planet::audio::flac::impl::impl(
        std::span<std::byte const> f, std::source_location const &l)
: loc{l}, flacdata{f} {
    decoder = FLAC__stream_decoder_new();
    if (not decoder) {
        throw felspar::stdexcept::runtime_error{
                "Failed to create FLAC decoder", loc};
    }
    if (FLAC__stream_decoder_init_stream(
                decoder, read_callback, nullptr, nullptr, nullptr, nullptr,
                write_callback, metadata_callback, error_callback, this)
        != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        FLAC__stream_decoder_delete(decoder);
        throw felspar::stdexcept::runtime_error{
                "Failed to initialise FLAC decoder", loc};
    }
    if (not FLAC__stream_decoder_process_until_end_of_metadata(decoder)) {
        auto const message = error ? error : "Not FLAC audio data";
        FLAC__stream_decoder_delete(decoder);
        throw felspar::stdexcept::runtime_error{message, loc};
    }
    if (not stream_info) {
        FLAC__stream_decoder_delete(decoder);
        throw felspar::stdexcept::runtime_error{
                "FLAC file has no STREAMINFO block", loc};
    }
}


planet::audio::flac::impl::~impl() {
    if (decoder) {
        FLAC__stream_decoder_finish(decoder);
        FLAC__stream_decoder_delete(decoder);
    }
}


FLAC__StreamDecoderReadStatus planet::audio::flac::impl::read_callback(
        FLAC__StreamDecoder const *,
        FLAC__byte buffer[],
        std::size_t *const bytes,
        void *const client_data) {
    auto &self = *static_cast<impl *>(client_data);
    auto const remaining = self.flacdata.size() - self.read_position;
    if (remaining == 0) {
        *bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
    auto const count = std::min(*bytes, remaining);
    std::copy(
            self.flacdata.begin() + self.read_position,
            self.flacdata.begin() + self.read_position + count,
            reinterpret_cast<std::byte *>(buffer));
    self.read_position += count;
    *bytes = count;
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}


FLAC__StreamDecoderWriteStatus planet::audio::flac::impl::write_callback(
        FLAC__StreamDecoder const *,
        FLAC__Frame const *const frame,
        FLAC__int32 const *const buffer[],
        void *const client_data) {
    auto &self = *static_cast<impl *>(client_data);
    auto const samples = static_cast<std::size_t>(frame->header.blocksize);
    /**
     * Samples are signed integers with `bits_per_sample` of resolution; scale
     * by the full-scale magnitude to land back in the `[-1, 1)` float range.
     */
    auto const scale =
            1.0f
            / static_cast<float>(
                    std::int64_t{1} << (frame->header.bits_per_sample - 1));
    self.output.ensure_length(samples * stereo_buffer::channels);
    for (std::size_t sample{}; sample < samples; ++sample) {
        self.output.at(sample * stereo_buffer::channels + 0) =
                static_cast<float>(buffer[0][sample]) * scale;
        self.output.at(sample * stereo_buffer::channels + 1) =
                static_cast<float>(buffer[1][sample]) * scale;
    }
    self.frame_samples = samples;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


void planet::audio::flac::impl::metadata_callback(
        FLAC__StreamDecoder const *,
        FLAC__StreamMetadata const *const metadata,
        void *const client_data) {
    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        static_cast<impl *>(client_data)->stream_info =
                metadata->data.stream_info;
    }
}


void planet::audio::flac::impl::error_callback(
        FLAC__StreamDecoder const *,
        FLAC__StreamDecoderErrorStatus const status,
        void *const client_data) {
    static_cast<impl *>(client_data)->error =
            FLAC__StreamDecoderErrorStatusString[status];
}


felspar::coro::generator<
        planet::audio::buffer_storage<planet::audio::sample_clock, 2>>
        planet::audio::flac::impl::stereo() {
    if (stream_info->channels != stereo_buffer::channels) {
        throw felspar::stdexcept::runtime_error{
                "This FLAC file is not stereo", loc};
    } else if (stream_info->sample_rate != samples_per_second) {
        throw felspar::stdexcept::runtime_error{
                "The sample rate is wrong. Should be 48kHz and is "
                        + std::to_string(stream_info->sample_rate) + "Hz",
                loc};
    }
    while (FLAC__stream_decoder_get_state(decoder)
           != FLAC__STREAM_DECODER_END_OF_STREAM) {
        frame_samples = 0;
        if (not FLAC__stream_decoder_process_single(decoder)) {
            throw felspar::stdexcept::runtime_error{
                    error ? error : "FLAC decoding failed", loc};
        }
        if (frame_samples) {
            co_yield output.first(frame_samples * stereo_buffer::channels);
        }
    }
}


/// ## `planet::audio::flac`


namespace {
    /**
     * The 64-bit big-endian STREAMINFO word that packs the sample rate (20
     * bits), the channel count (3 bits, stored as `count - 1`), the bit depth
     * (5 bits) and the total sample count (36 bits). STREAMINFO is always the
     * first metadata block, so it begins at byte 8 -- after the four byte
     * "fLaC" marker and the four byte metadata block header -- and the word
     * sits at offset 10 within the block.
     */
    std::uint64_t flac_streaminfo_word(
            std::span<std::byte const> const filedata,
            std::source_location const &loc) {
        if (filedata.size() < 4 or filedata[0] != std::byte{'f'}
            or filedata[1] != std::byte{'L'} or filedata[2] != std::byte{'a'}
            or filedata[3] != std::byte{'C'}) {
            throw felspar::stdexcept::runtime_error{"Not a FLAC file", loc};
        }
        std::size_t constexpr word_offset = 8 + 10;
        if (filedata.size() < word_offset + 8) {
            throw felspar::stdexcept::runtime_error{
                    "FLAC STREAMINFO block is truncated", loc};
        }
        return felspar::parse::binary::be::unchecked_extract<std::uint64_t>(
                std::span<std::byte const, 8>{filedata.data() + word_offset, 8});
    }
}


planet::audio::flac::flac(std::vector<std::byte> f, std::source_location const l)
: m_filedata{std::move(f)}, loc{l} {}


felspar::coro::generator<
        planet::audio::buffer_storage<planet::audio::sample_clock, 2>>
        planet::audio::flac::stereo() {
    impl decoder{m_filedata, loc};
    for (auto s = decoder.stereo(); auto p = s.next();) {
        co_yield std::move(*p);
    }
}


std::size_t planet::audio::flac::channels() const {
    /**
     * The channel count is a 3-bit field stored as `count - 1`, sitting just
     * below the 20-bit sample rate at the top of the STREAMINFO word.
     */
    std::uint64_t constexpr channels_mask = 0x7;
    auto const word = flac_streaminfo_word(m_filedata, loc);
    return static_cast<std::size_t>(((word >> 41) bitand channels_mask) + 1);
}


std::size_t planet::audio::flac::sample_rate() const {
    /**
     * The sample rate is the top 20 bits of the STREAMINFO word.
     */
    auto const word = flac_streaminfo_word(m_filedata, loc);
    return static_cast<std::size_t>(word >> 44);
}


planet::audio::sample_clock planet::audio::flac::duration() const {
    /**
     * `total_samples` is the low 36 bits of the STREAMINFO word.
     */
    std::uint64_t constexpr total_samples_mask = 0xF'FFFF'FFFF;
    auto const word = flac_streaminfo_word(m_filedata, loc);
    return sample_clock{
            static_cast<std::int64_t>(word bitand total_samples_mask)};
}


/// ## `planet::audio::ogg::impl`


struct planet::audio::ogg::impl {
    impl(std::span<std::byte>, std::source_location const &);
    ~impl();

    vorbis_info vi;
    vorbis_comment vc;
    std::source_location loc;

    felspar::coro::generator<
            planet::audio::buffer_storage<planet::audio::sample_clock, 2>>
            stereo();

  private:
    std::span<std::byte> ogg;
    felspar::coro::generator<ogg_packet> packets;
    felspar::coro::generator<ogg_packet> vorbis_packets();
};


planet::audio::ogg::impl::impl(
        std::span<std::byte> o, std::source_location const &l)
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


namespace {
    /**
     * Locate the Vorbis identification header -- the first packet in the
     * stream -- and return the index of its leading 0x01 marker. The marker is
     * followed by "vorbis", a 4-byte version, the 1-byte channel count and the
     * 4-byte little-endian sample rate, so the caller can read those fields at
     * fixed offsets from the returned index.
     */
    std::size_t vorbis_identification_header(
            std::span<std::byte const> const filedata,
            std::source_location const &loc) {
        for (std::size_t i{}; i + 16 <= filedata.size(); ++i) {
            if (filedata[i] == std::byte{0x01}
                and filedata[i + 1] == std::byte{'v'}
                and filedata[i + 2] == std::byte{'o'}
                and filedata[i + 3] == std::byte{'r'}
                and filedata[i + 4] == std::byte{'b'}
                and filedata[i + 5] == std::byte{'i'}
                and filedata[i + 6] == std::byte{'s'}) {
                return i;
            }
        }
        throw felspar::stdexcept::runtime_error{
                "No Vorbis identification header found", loc};
    }
}


planet::audio::ogg::ogg(std::vector<std::byte> o, std::source_location const l)
: m_filedata{std::move(o)}, loc{l} {}


felspar::coro::generator<
        planet::audio::buffer_storage<planet::audio::sample_clock, 2>>
        planet::audio::ogg::stereo() {
    impl decoder{m_filedata, loc};
    for (auto s = decoder.stereo(); auto p = s.next();) {
        co_yield std::move(*p);
    }
}


std::size_t planet::audio::ogg::channels() const {
    /**
     * The single byte channel count follows the 4-byte version field after the
     * header's 7-byte signature.
     */
    auto const header = vorbis_identification_header(m_filedata, loc);
    return std::to_integer<std::size_t>(m_filedata[header + 11]);
}


std::size_t planet::audio::ogg::sample_rate() const {
    /**
     * The 4-byte little-endian sample rate follows the channel count.
     */
    auto const header = vorbis_identification_header(m_filedata, loc);
    return static_cast<std::size_t>(
            felspar::parse::binary::le::unchecked_extract<std::uint32_t>(
                    std::span<std::byte const, 4>{
                            m_filedata.data() + header + 12, 4}));
}


planet::audio::sample_clock planet::audio::ogg::duration() const {
    /**
     * The last OGG page's granule position holds the total PCM sample count.
     * Scan backward for the last "OggS" capture pattern and read the 8-byte
     * little-endian granule position at offset +6.
     */
    for (auto i = static_cast<std::ptrdiff_t>(m_filedata.size()) - 27; i >= 0;
         --i) {
        if (m_filedata[i] == std::byte{'O'}
            and m_filedata[i + 1] == std::byte{'g'}
            and m_filedata[i + 2] == std::byte{'g'}
            and m_filedata[i + 3] == std::byte{'S'}) {
            auto const granule =
                    felspar::parse::binary::le::unchecked_extract<std::int64_t>(
                            std::span<std::byte const, 8>{
                                    m_filedata.data() + i + 6, 8});
            return sample_clock{granule};
        }
    }
    throw felspar::stdexcept::runtime_error{"No OggS block found", loc};
}
