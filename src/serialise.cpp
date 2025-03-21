#include <planet/log.hpp>
#include <planet/serialise/exceptions.hpp>
#include <planet/serialise/felspar.hpp>
#include <planet/serialise/load_buffer.hpp>
#include <planet/serialise/muxing.hpp>
#include <planet/serialise/save_buffer.hpp>
#include <planet/serialise/string.hpp>

#include <felspar/memory/hexdump.hpp>

#include <cstring>
#include <limits>
#include <mutex>
#include <ostream>
#include <string>
#include <utility>


/// ## `planet::serialise::demuxer`


planet::serialise::demuxer::demuxer() : id{"demuxer"} {}
planet::serialise::demuxer::demuxer(std::string_view const n)
: id{n, id::suffix::no} {}


void planet::serialise::demuxer::start_manager() {
    manager.post(*this, &demuxer::manage_simulation_subscriptions);
}


auto planet::serialise::demuxer::queue_for(std::string_view const n)
        -> queue::pmc<message> & {
    if (auto pos = subscribers.find(n); pos == subscribers.end()) {
        planet::log::debug("New subscriber for", n, "in demuxer", name());
        return subscribers
                .emplace(
                        std::piecewise_construct, std::forward_as_tuple(n),
                        std::forward_as_tuple())
                .first->second;
    } else {
        return pos->second;
    }
}


felspar::io::warden::task<void>
        planet::serialise::demuxer::manage_simulation_subscriptions() {
    try {
        while (true) {
            auto processing = co_await acquire();
            for (auto &&b : processing) {
                planet::serialise::load_buffer lb{b.cmemory()};
                while (not lb.empty()) {
                    auto box = planet::serialise::expect_box(lb);
                    queue_for(box.name).push({std::move(box), b});
                    ++enqueued;
                }
            }
        }
    } catch (...) { std::terminate(); }
}


void planet::serialise::demuxer::send(shared_bytes b) {
    push(std::move(b));
    ++sends;
}


/// ## `planet::serialise::box`


void planet::serialise::box::check_name_or_throw(
        std::string_view expected, felspar::source_location const &loc) const {
    if (name != expected) {
        throw felspar::stdexcept::runtime_error{
                "Unexpected box name\n"
                "Got '" + std::string{name}
                        + "' and expected '" + std::string{expected} + "'",
                loc};
    }
}


void planet::serialise::box::check_empty_or_throw(
        felspar::source_location const &loc) const {
    if (not content.empty()) { throw box_not_empty{*this, loc}; }
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
    auto const bytes =
            felspar::parse::binary::be::extract<std::uint64_t>(buffer, loc);
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
    buffer.ensure_length(written + b);
    return std::exchange(written, written + b);
}


std::span<std::byte>
        planet::serialise::save_buffer::allocate(std::size_t const b) {
    allocate_offset(b);
    return {buffer.memory().data() + written - b, b};
}


auto planet::serialise::save_buffer::complete() -> shared_bytes {
    return buffer.first(std::exchange(written, 0));
}


void planet::serialise::save_buffer::append_size_t(std::size_t const ss) {
    auto const u64 = std::uint64_t{ss};
    felspar::parse::binary::be::unchecked_insert(allocate_for(u64), u64);
}
void planet::serialise::save_buffer::append(std::span<std::byte const> sv) {
    auto const s = allocate(sv.size());
    std::memcpy(s.data(), sv.data(), sv.size());
}
void planet::serialise::save_buffer::append(std::span<char const> sc) {
    append(std::as_bytes(sc));
}
void planet::serialise::save_buffer::append(std::string_view const sv) {
    append(std::as_bytes(std::span{sv}));
}


/// ## `planet::serialise::marker`


std::string_view planet::serialise::to_string(marker const m) {
    auto const v = static_cast<std::uint8_t>(m);
    if (v > 0 and v < 0x80) {
        return "box";
    } else {
        switch (m) {
        case marker::empty: return "empty";

        case marker::std_byte_array: return "std::byte[]";
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

        case marker::poly_list: return "[]";

        case marker::u16le: return "u16le";
        case marker::i16le: return "i16le";
        case marker::u32le: return "u32le";
        case marker::i32le: return "i32le";
        case marker::u64le: return "u64le";
        case marker::i64le: return "i64le";
        case marker::u128le: return "u128le";
        case marker::i128le: return "i128le";

        case marker::f16le: return "f16le";
        case marker::f32le: return "f32le";
        case marker::f64le: return "f64le";
        case marker::f80le: return "f80le";
        case marker::f128le: return "f128le";

        case marker::u8string8: return "u8string8";

        case marker::u16string8be: return "u16string8be";
        case marker::u32string8be: return "u32string8be";

        case marker::u16string8le: return "u16string8le";
        case marker::u32string8le: return "u32string8le";
        }
    }
    return "Unknown marker";
}


/// ## Types in `std::`


void planet::serialise::detail::save_string(
        save_buffer &ab,
        std::span<std::byte const> const sv,
        std::size_t const charsize) {
    ab.append(marker_for_character_size(charsize));
    ab.append_size_t(sv.size() / charsize);
    ab.append(sv);
}
void planet::serialise::load(load_buffer &lb, std::string &s) {
    lb.check_marker(marker::u8string8);
    auto const sz = lb.extract_size_t();
    auto const b = lb.split(sz);
    s = {reinterpret_cast<char const *>(b.data()), b.size()};
}
void planet::serialise::load(load_buffer &lb, std::wstring &s) {
    static constexpr auto charsize = sizeof(std::wstring::value_type);
    static constexpr auto marker = marker_for_character_size(charsize);
    lb.check_marker(marker);
    auto const sz = lb.extract_size_t();
    auto const b = lb.split(sz * charsize);
    s = {reinterpret_cast<std::wstring::value_type const *>(b.data()), sz};
}
void planet::serialise::load(load_buffer &lb, std::string_view &s) {
    lb.check_marker(marker::u8string8);
    auto const sz = lb.extract_size_t();
    auto const b = lb.split(sz);
    s = {reinterpret_cast<char const *>(b.data()), b.size()};
}


/// ## Types in `felspar::`


void planet::serialise::save(
        save_buffer &ab, felspar::source_location const &loc) {
    ab.save_box(
            "_s:sl", std::string_view{loc.file_name()},
            std::string_view{loc.function_name()}, loc.line(), loc.column());
}
auto const fsl = planet::log::format("_s:sl", [](auto &os, auto &box) {
    std::string file_name, function_name;
    unsigned line, column;
    box.named("_s:sl", file_name, function_name, line, column);
    os << function_name << '@' << file_name << ':' << line << ':' << column;
});


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
        box const &b, felspar::source_location const &loc)
: serialisation_error{
          "The box was not empty after loading all data from it\n"
          "Box content: "
                  + std::string{felspar::memory::hexdump(b.content.cmemory())},
          loc} {}


planet::serialise::buffer_not_big_enough::buffer_not_big_enough(
        std::size_t const wanted,
        std::size_t const got,
        felspar::source_location const &loc)
: serialisation_error{
          "There was not enough data in the buffer to read the requested "
          "value\n"
          "Wanted "
                  + std::to_string(wanted) + " bytes and buffer only contains "
                  + std::to_string(got) + " bytes",
          loc} {}


planet::serialise::invalid_charsize::invalid_charsize(
        std::size_t const charsize, felspar::source_location const &loc)
: serialisation_error{
          "Invalid UTF character size in bytes\n"
          "Expected 1, 2 or 4. Got "
                  + std::to_string(charsize),
          loc} {}
void planet::serialise::detail::throw_invalid_charsize(
        std::size_t const charsize, felspar::source_location const &loc) {
    throw invalid_charsize{charsize, loc};
}


planet::serialise::unsupported_version_number::unsupported_version_number(
        box const &b,
        std::uint8_t const highest,
        felspar::source_location const &loc)
: serialisation_error{
          "Unsupported version number in box\n"
          "Unsupported version "
                  + std::to_string(b.version) + ", and only up to "
                  + std::to_string(highest) + " is supported",
          loc} {
    inside_box(b.name);
}


planet::serialise::wanted_boolean::wanted_boolean(
        std::span<std::byte const> remaining,
        marker const m,
        felspar::source_location const &loc)
: serialisation_error{
          std::string{"Expected b_true or b_false for a boolean\n"
                      "Got "}
                  + std::string{to_string(m)},
          remaining, loc} {}


planet::serialise::wanted_box::wanted_box(
        std::span<std::byte const> remaining,
        marker const m,
        felspar::source_location const &loc)
: serialisation_error{
          std::string{"Expected a box marker\nGot "}
                  + std::string{to_string(m)},
          remaining, loc} {}


planet::serialise::wrong_marker::wrong_marker(
        marker const expected,
        marker const got,
        felspar::source_location const &loc)
: serialisation_error{
          std::string{"The wrong type marker is in the save file\n"
                      "Expected "}
                  + std::string{to_string(expected)} + " and got "
                  + std::string{to_string(got)} + " ("
                  + std::to_string(static_cast<std::uint8_t>(got)) + ")",
          loc} {}
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
