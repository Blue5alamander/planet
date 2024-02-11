#include <planet/comms/signal.hpp>

#include <felspar/io/write.hpp>


/// ## `planet::comms::internal`


planet::comms::internal::internal(felspar::io::warden &w) : warden{w} {}


std::size_t planet::comms::internal::write(std::span<std::byte const> const b) {
    return felspar::io::write_some(
            pipe.write.native_handle(), b.data(), b.size());
}


/// ## `planet::comms::signal`


planet::comms::signal::signal(felspar::io::warden &w) : pipe{w} {}


void planet::comms::signal::send(std::byte const b) {
    std::array<std::byte, 1> const sig{b};
    if (pipe.write(sig) != 1u) { std::terminate(); }
}
