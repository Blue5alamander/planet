#pragma once


#include <felspar/parse/concepts.hpp>
#include <felspar/parse/endian.hpp>

#include <cstdint>
#include <string_view>


namespace planet::serialise {


    /// ## Type markers
    enum class marker : std::uint8_t {
        empty = 0x00,

        /**
         * Between 0x01 and 0x7f are boxes (specifying the box name length in
         * bytes)
         */

        std_byte_array = 0x80,
        u8,
        i8,
        u16be,
        i16be,
        u32be,
        i32be,
        u64be,
        i64be,
        u128be,
        i128be,
        b_true,
        b_false,

        f16be = 0x91,
        f32be,
        f64be,
        f80be,
        f128be,

        poly_list = 0xa0,

        u16le = 0xa3,
        i16le,
        u32le,
        i32le,
        u64le,
        i64le,
        u128le,
        i128le,

        f16le = 0xb1,
        f32le,
        f64le,
        f80le,
        f128le,

        u8string8 = 0xc3,

        u16string8be = 0xc7,
        u32string8be = 0xcb,

        u16string8le = 0xe7,
        u32string8le = 0xeb,
    };


    /// ### Return the marker as a string representation
    std::string_view to_string(marker);


    /// ### Return the marker for a given low-level type
    template<typename T>
    constexpr marker marker_for();

    template<felspar::parse::concepts::unsigned_integral T>
    constexpr marker marker_for() {
        if constexpr (sizeof(T) == 1) {
            return marker::u8;
        } else if constexpr (
                felspar::parse::endian::native == felspar::parse::endian::big) {
            switch (sizeof(T)) {
            case 2: return marker::u16be;
            case 4: return marker::u32be;
            case 8: return marker::u64be;
            case 16: return marker::u128be;
            }
        } else {
            switch (sizeof(T)) {
            case 2: return marker::u16le;
            case 4: return marker::u32le;
            case 8: return marker::u64le;
            case 16: return marker::u128le;
            }
        }
    }

    template<felspar::parse::concepts::signed_integral T>
    constexpr marker marker_for() {
        if constexpr (sizeof(T) == 1) {
            return marker::i8;
        } else if constexpr (
                felspar::parse::endian::native == felspar::parse::endian::big) {
            switch (sizeof(T)) {
            case 2: return marker::i16be;
            case 4: return marker::i32be;
            case 8: return marker::i64be;
            case 16: return marker::i128be;
            }
        } else {
            switch (sizeof(T)) {
            case 2: return marker::i16le;
            case 4: return marker::i32le;
            case 8: return marker::i64le;
            case 16: return marker::i128le;
            }
        }
    }

    template<felspar::parse::concepts::floating_point T>
    constexpr marker marker_for() {
        if constexpr (felspar::parse::endian::native == felspar::parse::endian::big) {
            switch (sizeof(T)) {
            case 4: return marker::f32be;
            case 8: return marker::f64be;
            case 16: return marker::f128be;
            }
        } else {
            switch (sizeof(T)) {
            case 4: return marker::f32le;
            case 8: return marker::f64le;
            case 16: return marker::f128le;
            }
        }
    }

    /// #### Marker for a character size
    namespace detail {
        [[noreturn]] void throw_invalid_charsize(
                std::size_t const charsize, std::source_location const &);
    }
    constexpr marker marker_for_character_size(
            std::size_t const charsize,
            std::source_location const &loc = std::source_location::current()) {
        switch (charsize) {
        case 1: return marker::u8string8;
        case 2:
            if constexpr (
                    felspar::parse::endian::native
                    == felspar::parse::endian::big) {
                return marker::u16string8be;
            } else {
                return marker::u16string8le;
            }
        case 4:
            if constexpr (
                    felspar::parse::endian::native
                    == felspar::parse::endian::big) {
                return marker::u32string8be;
            } else {
                return marker::u32string8le;
            }
        default: detail::throw_invalid_charsize(charsize, loc);
        }
    }


    /// ### Classify this marker

    constexpr bool is_box_marker(marker const m) {
        auto const mm = static_cast<std::uint8_t>(m);
        return mm > 0 and mm < 0x80;
    }


    /// ### Endian modes

    constexpr bool is_endian(marker const m) {
        switch (m) {
        case marker::empty:

        case marker::std_byte_array:

        case marker::u8:
        case marker::i8:

        case marker::b_true:
        case marker::b_false:

        case marker::poly_list:

        case marker::u8string8: return false;

        case marker::u16be:
        case marker::i16be:
        case marker::u32be:
        case marker::i32be:
        case marker::u64be:
        case marker::i64be:
        case marker::u128be:
        case marker::i128be:

        case marker::f16be:
        case marker::f32be:
        case marker::f64be:
        case marker::f80be:
        case marker::f128be:

        case marker::u16le:
        case marker::i16le:
        case marker::u32le:
        case marker::i32le:
        case marker::u64le:
        case marker::i64le:
        case marker::u128le:
        case marker::i128le:

        case marker::f16le:
        case marker::f32le:
        case marker::f64le:
        case marker::f80le:
        case marker::f128le:

        case marker::u16string8be:
        case marker::u32string8be:
        case marker::u16string8le:
        case marker::u32string8le: return true;
        }
        // gcc is confused about this
        return false;
    }

    constexpr marker other_endian(marker const m) {
        switch (m) {
        case marker::empty:

        case marker::std_byte_array:

        case marker::u8:
        case marker::i8:

        case marker::b_true:
        case marker::b_false:

        case marker::poly_list:

        case marker::u8string8: return m;

        case marker::u16be:
        case marker::i16be:
        case marker::u32be:
        case marker::i32be:
        case marker::u64be:
        case marker::i64be:
        case marker::u128be:
        case marker::i128be:

        case marker::f16be:
        case marker::f32be:
        case marker::f64be:
        case marker::f80be:
        case marker::f128be:
            return marker{static_cast<std::uint8_t>(
                    static_cast<std::uint8_t>(m) - 0x80 + 0xa0)};

        case marker::u16le:
        case marker::i16le:
        case marker::u32le:
        case marker::i32le:
        case marker::u64le:
        case marker::i64le:
        case marker::u128le:
        case marker::i128le:

        case marker::f16le:
        case marker::f32le:
        case marker::f64le:
        case marker::f80le:
        case marker::f128le:
            return marker{static_cast<std::uint8_t>(
                    static_cast<std::uint8_t>(m) - 0xa0 + 0x80)};

        case marker::u16string8be:
        case marker::u32string8be:
            return marker{static_cast<std::uint8_t>(
                    static_cast<std::uint8_t>(m) - 0xc0 + 0xe0)};

        case marker::u16string8le:
        case marker::u32string8le:
            return marker{static_cast<std::uint8_t>(
                    static_cast<std::uint8_t>(m) - 0xe0 + 0xc0)};
        }
        // gcc is confused about this
        return m;
    }


}
