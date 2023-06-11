#include <planet/comms/signal.hpp>


/// ## `planet::comms::internal`


planet::comms::internal::internal(felspar::io::warden &w) : warden{w} {}


std::size_t planet::comms::internal::write(std::span<std::byte const> const b) {
    if (auto r = ::write(pipe.write.native_handle(), b.data(), b.size());
        r >= 0) {
        return r;
    } else {
        throw felspar::stdexcept::system_error{
                errno, std::system_category(), "Writing to pipe"};
    }
}


/// ## `planet::comms::signal`


planet::comms::signal::signal(felspar::io::warden &w) : pipe{w} {}


void planet::comms::signal::send(std::byte const b) {
    std::array<std::byte, 1> const sig{b};
    if (pipe.write(sig) != 1u) { std::terminate(); }
}
