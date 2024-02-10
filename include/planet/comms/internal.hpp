#pragma once


#include <felspar/io/warden.hpp>


namespace planet::comms {


    /// ## Internal communications
    class internal {
        /// Warden used for message passing
        felspar::io::warden &warden;

        /// Thread data interchange
        felspar::io::pipe pipe = warden.create_pipe();

      public:
        internal(felspar::io::warden &);

        /// ### Synchronous send of data
        std::size_t write(std::span<std::byte const>);

        /// ### Asynchronous read of data
        template<typename B>
        auto read_some(B &&b) {
            return warden.read_some(pipe.read, std::forward<B>(b));
        }
    };


}
