#include <planet/comms/inproc.hpp>
#include <planet/comms/internal.hpp>
#include <planet/comms/signal.hpp>

#include <felspar/io/write.hpp>


/// ## `planet::comms::inproc::data`


planet::comms::inproc::data::data(
        std::string_view const n, felspar::io::warden &w)
: demuxer{n}, signalling{w} {
    start_manager();
}


auto planet::comms::inproc::data::subscribe(std::string_view const n)
        -> queue::pmc<serialise::demuxer::message>::consumer {
    return queue_for(n).values();
}


void planet::comms::inproc::data::do_push(serialise::shared_bytes b) {
    queue.push(std::move(b));
    signalling.send({});
    ++pushes;
}


auto planet::comms::inproc::data::acquire()
        -> felspar::io::warden::task<std::span<serialise::shared_bytes>> {
    auto s = queue.consume();
    while (s.empty()) {
        std::array<std::byte, 16> buffer;
        co_await signalling.read_some(buffer);
        s = queue.consume();
    }
    deliveries += s.size();
    co_return s;
}


/// ## `planet::comms::internal`


planet::comms::internal::internal(
        felspar::io::warden &w, std::source_location const &loc)
: warden{w}, pipe{warden.create_pipe(loc)} {}


std::size_t planet::comms::internal::write(std::span<std::byte const> const b) {
    return felspar::io::write_some(
            pipe.write.native_handle(), b.data(), b.size());
}


/// ## `planet::comms::signal`


planet::comms::signal::signal(
        felspar::io::warden &w, std::source_location const &loc)
: warden{w}, pipe{warden.create_pipe(loc)} {}


void planet::comms::signal::send(std::byte const b) {
    std::array<std::byte, 1> const sig{b};
    if (felspar::io::write_some(
                pipe.write.native_handle(), sig.data(), sig.size())
        != 1u) {
        std::terminate();
    }
}
