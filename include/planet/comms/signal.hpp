#pragma once


#include <planet/comms/internal.hpp>


namespace planet::comms {


    /// ## Internal signalling
    class signal {
        internal pipe;

      public:
        signal(felspar::io::warden &);

        /// ### Send a signal to the far end of the pipe
        void send(std::byte);

        /// ### Read signals
        auto read_some(std::span<std::byte> b) { return pipe.read_some(b); }
    };


}
