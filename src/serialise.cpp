#include <planet/serialise/exceptions.hpp>
#include <planet/serialise/load.hpp>
#include <planet/serialise/save_buffer.hpp>

#include <cstring>
#include <ostream>


/// ## `planet::serialise::box`


void planet::serialise::box::check_name_or_throw(
        std::string_view expected) const {
    if (name != expected) {
        throw felspar::stdexcept::runtime_error{"Unexpected box name"};
    }
}


void planet::serialise::box::check_empty_or_throw(
        felspar::source_location const &loc) const {
    if (not content.empty()) { throw box_not_empty{loc}; }
}


/// ## `planet::serialise::save_buffer`


planet::serialise::save_buffer::save_buffer() : buffer(1 << 20) {}


std::size_t
        planet::serialise::save_buffer::allocate_offset(std::size_t const b) {
    if (written + b > buffer.size()) {
        throw felspar::stdexcept::logic_error{"Not implemented"};
    } else {
        return std::exchange(written, written + b);
    }
}


std::span<std::byte>
        planet::serialise::save_buffer::allocate(std::size_t const b) {
    allocate_offset(b);
    return {buffer.data() + written - b, b};
}


felspar::memory::shared_bytes planet::serialise::save_buffer::complete() {
    return buffer.consume_first(written);
}


void planet::serialise::save_buffer::append(std::string_view const sv) {
    append(sv.size());
    auto const s = allocate(sv.size());
    std::memcpy(s.data(), sv.data(), sv.size());
}


/// ## `planet::serialise::marker`


std::string_view planet::serialise::to_string(marker const m) {
    auto const v = static_cast<std::uint8_t>(m);
    if (v > 0 and v < 0x80) {
        return "box";
    } else {
        switch (m) {
        case marker::empty: return "empty";
        case marker::binary: return "binary";

        case marker::u8: return "u8";
        case marker::i8: return "i8";
        case marker::u16: return "u16";
        case marker::i16: return "i16";
        case marker::u32: return "u32";
        case marker::i32: return "i32";
        case marker::u64: return "u64";
        case marker::i64: return "i64";
        case marker::u128: return "u128";
        case marker::i128: return "i128";
        case marker::size_t: return "size_t";

        case marker::f16: return "f16";
        case marker::f32: return "f32";
        case marker::f64: return "f64";
        case marker::f80: return "f80";
        case marker::f128: return "f128";

        case marker::string: return "string";
        }
    }
    return "Unknown marker";
}


/// ## `planet::serialise::serialisation_error` and related sub-classes


planet::serialise::serialisation_error::serialisation_error(
        std::string m, felspar::source_location const &loc)
: felspar::stdexcept::runtime_error{std::move(m), loc} {}


planet::serialise::box_not_empty::box_not_empty(
        felspar::source_location const &loc)
: serialisation_error{
        "The box was not empty after loading all data from it", loc} {}
