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


/// ## `planet::serialise::serialisation_error` and related sub-classes


planet::serialise::serialisation_error::serialisation_error(
        std::string m, felspar::source_location const &loc)
: felspar::stdexcept::runtime_error{std::move(m), loc} {}


planet::serialise::box_not_empty::box_not_empty(
        felspar::source_location const &loc)
: serialisation_error{
        "The box was not empty after loading all data from it", loc} {}
