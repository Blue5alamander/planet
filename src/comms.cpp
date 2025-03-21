#include <planet/comms/internal.hpp>
#include <planet/comms/signal.hpp>

#include <felspar/io/write.hpp>


/// ## `planet::comms::internal`


planet::comms::internal::internal(
        felspar::io::warden &w, felspar::source_location const &loc)
: warden{w}, pipe{warden.create_pipe(loc)} {}


std::size_t planet::comms::internal::write(std::span<std::byte const> const b) {
    return felspar::io::write_some(
            pipe.write.native_handle(), b.data(), b.size());
}


/// ## `planet::comms::signal`


planet::comms::signal::signal(
        felspar::io::warden &w, felspar::source_location const &loc)
: warden{w}, pipe{warden.create_pipe(loc)} {}


void planet::comms::signal::send(std::byte const b) {
    std::array<std::byte, 1> const sig{b};
    if (felspar::io::write_some(
                pipe.write.native_handle(), sig.data(), sig.size())
        != 1u) {
        std::terminate();
    }
}
