#pragma once


#include <felspar/parse/concepts.hpp>

#include <string_view>


namespace planet::serialise {


    /// ## Type markers
    enum class marker : std::uint8_t {
        empty = 0x00,

        // Between 0x01 and 0x7f are boxes (specifying the box name length in
        // bytes)

        binary = 0x80,

        u8 = 0x81,
        i8,
        u16,
        i16,
        u32,
        i32,
        u64,
        i64,
        u128,
        i128,
        b_true,
        b_false,

        f16 = 0x91,
        f32,
        f64,
        f80,
        f128,

        string = 0xa1,
    };


    /// ### Return the marker as a string representation
    std::string_view to_string(marker);


    /// ### Return the marker for a given low-level type
    template<typename T>
    constexpr marker marker_for();

    template<felspar::parse::concepts::unsigned_integral T>
    constexpr marker marker_for() {
        switch (sizeof(T)) {
        case 1: return marker::u8;
        case 2: return marker::u16;
        case 4: return marker::u32;
        case 8: return marker::u64;
        case 16: return marker::u128;
        }
    }

    template<felspar::parse::concepts::signed_integral T>
    constexpr marker marker_for() {
        switch (sizeof(T)) {
        case 1: return marker::i8;
        case 2: return marker::i16;
        case 4: return marker::i32;
        case 8: return marker::i64;
        case 16: return marker::i128;
        }
    }

    template<>
    constexpr marker marker_for<std::string_view>() {
        return marker::string;
    }


    constexpr bool is_box_marker(marker const m) {
        auto const mm = static_cast<std::uint8_t>(m);
        return mm > 1 and mm < 0x80;
    }


}
