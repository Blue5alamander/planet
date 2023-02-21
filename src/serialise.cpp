#include <planet/serialise/exceptions.hpp>
#include <planet/serialise/load_buffer.hpp>
#include <planet/serialise/save_buffer.hpp>

#include <felspar/memory/hexdump.hpp>

#include <cstring>
#include <limits>
#include <ostream>
#include <string>
#include <utility>


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


/// ## `planet::serialise::load_buffer`


namespace {
    /// `std::in_range` and save comparisons are not available in the Android
    /// compiler :-(
    template<class T, class U>
    constexpr bool cmp_equal(T t, U u) noexcept {
        using UT = std::make_unsigned_t<T>;
        using UU = std::make_unsigned_t<U>;
        if constexpr (std::is_signed_v<T> == std::is_signed_v<U>) {
            return t == u;
        } else if constexpr (std::is_signed_v<T>) {
            return t < 0 ? false : UT(t) == u;
        } else {
            return u < 0 ? false : t == UU(u);
        }
    }

    template<class T, class U>
    constexpr bool cmp_not_equal(T t, U u) noexcept {
        return not cmp_equal(t, u);
    }

    template<class T, class U>
    constexpr bool cmp_less(T t, U u) noexcept {
        using UT = std::make_unsigned_t<T>;
        using UU = std::make_unsigned_t<U>;
        if constexpr (std::is_signed_v<T> == std::is_signed_v<U>) {
            return t < u;
        } else if constexpr (std::is_signed_v<T>) {
            return t < 0 ? true : UT(t) < u;
        } else {
            return u < 0 ? false : t < UU(u);
        }
    }

    template<class T, class U>
    constexpr bool cmp_greater(T t, U u) noexcept {
        return cmp_less(u, t);
    }

    template<class T, class U>
    constexpr bool cmp_less_equal(T t, U u) noexcept {
        return not cmp_greater(t, u);
    }

    template<class T, class U>
    constexpr bool cmp_greater_equal(T t, U u) noexcept {
        return not cmp_less(t, u);
    }

    template<class R, class T>
    constexpr bool in_range(T t) noexcept {
        return cmp_greater_equal(t, std::numeric_limits<R>::min())
                and cmp_less_equal(t, std::numeric_limits<R>::max());
    }
}
std::size_t planet::serialise::load_buffer::extract_size_t(
        felspar::source_location const &loc) {
    auto const bytes = extract<std::uint64_t>(loc);
    if constexpr (sizeof(std::size_t) < sizeof(std::uint64_t)) {
        if (not in_range<std::size_t>(bytes)) {
            throw felspar::stdexcept::runtime_error{
                    "This save file is too large to load on this machine", loc};
        } else {
            return std::size_t(bytes);
        }
    } else {
        return bytes;
    }
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


void planet::serialise::save_buffer::append_size_t(std::size_t const ss) {
    append(std::uint64_t{ss});
}
void planet::serialise::save_buffer::append(std::span<std::byte const> sv) {
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

        case marker::std_byte: return "std::byte";
        case marker::u8: return "u8";
        case marker::i8: return "i8";
        case marker::u16be: return "u16be";
        case marker::i16be: return "i16be";
        case marker::u32be: return "u32be";
        case marker::i32be: return "i32be";
        case marker::u64be: return "u64be";
        case marker::i64be: return "i64be";
        case marker::u128be: return "u128be";
        case marker::i128be: return "i128be";
        case marker::b_false: return "b_false";
        case marker::b_true: return "b_true";

        case marker::f16be: return "f16be";
        case marker::f32be: return "f32be";
        case marker::f64be: return "f64be";
        case marker::f80be: return "f80be";
        case marker::f128be: return "f128be";

        case marker::string: return "string";

        case marker::std_byte_array: return "std::byte[]";
        }
    }
    return "Unknown marker";
}


/// ## `planet::serialise::serialisation_error`


planet::serialise::serialisation_error::serialisation_error(
        std::string m, felspar::source_location const &loc)
: felspar::stdexcept::runtime_error{std::move(m), loc} {}

planet::serialise::serialisation_error::serialisation_error(
        std::string m,
        std::span<std::byte const> r,
        felspar::source_location const &loc)
: felspar::stdexcept::runtime_error{std::move(m), loc},
  hexdump{felspar::memory::hexdump(r)} {}

char const *planet::serialise::serialisation_error::what() const noexcept {
    if (what_message.empty()) {
        return felspar::stdexcept::runtime_error::what();
    } else {
        return what_message.c_str();
    }
}

void planet::serialise::serialisation_error::inside_box(std::string_view sv) {
    if (what_message.empty()) {
        what_message = std::string{felspar::stdexcept::runtime_error::what()};
        if (not hexdump.empty()) {
            what_message.append("\nRemaining buffer: ");
            what_message.append(hexdump);
        }
        what_message.append("\nBox path: ");
        what_message.append(sv);
    } else {
        what_message.append(" <- ");
        what_message.append(sv);
    }
}


/// ## `planet::serialise::serialisation_error` sub-classes


planet::serialise::box_name_length::box_name_length(
        std::string_view box_name, felspar::source_location const &loc)
: serialisation_error{
        std::string{"The box name must be between 1 and 127 bytes long\n"
                    "Name tried is '"}
                + std::string{box_name} + '\'',
        loc} {}


planet::serialise::box_not_empty::box_not_empty(
        felspar::source_location const &loc)
: serialisation_error{
        "The box was not empty after loading all data from it", loc} {}


planet::serialise::buffer_not_big_enough::buffer_not_big_enough(
        std::size_t const wanted,
        std::size_t const got,
        felspar::source_location const &loc)
: serialisation_error{
        "There was not enough data in the buffer to read the requested value\n"
        "Wanted "
                + std::to_string(wanted) + " bytes and buffer only contains "
                + std::to_string(got) + " bytes",
        loc} {}


planet::serialise::wanted_boolean::wanted_boolean(
        std::span<std::byte const> remaining,
        marker const m,
        felspar::source_location const &loc)
: serialisation_error{
        std::string{"Expected b_true or b_false for a boolean\n"
                    "Got "}
                + std::string{to_string(m)},
        remaining, loc} {}


planet::serialise::wrong_marker::wrong_marker(
        std::span<std::byte const> remaining,
        marker const expected,
        marker const got,
        felspar::source_location const &loc)
: serialisation_error{
        std::string{"The wrong type marker is in the save file\n"
                    "Expected "}
                + std::string{to_string(expected)} + " and got "
                + std::string{to_string(got)} + " ("
                + std::to_string(static_cast<std::uint8_t>(got)) + ")",
        remaining, loc} {}
