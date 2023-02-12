#include <planet/serialise/save_buffer.hpp>

#include <cstring>
#include <ostream>


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
