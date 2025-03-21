#pragma once


#include <felspar/io/warden.hpp>


namespace planet::comms {


    /// ## Internal signalling
    class signal {
        felspar::io::warden &warden;
        felspar::io::pipe pipe;

      public:
        signal(felspar::io::warden &,
               felspar::source_location const & =
                       felspar::source_location::current());


        /// ### Send a signal to the far end of the pipe
        void send(std::byte);


        /// ### Read signals
        auto read_some(std::span<std::byte> b) {
            return warden.read_some(pipe.read, b);
        }
    };


}
