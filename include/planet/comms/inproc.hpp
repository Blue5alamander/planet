#pragma once


#include <planet/comms/signal.hpp>
#include <planet/queue/tspsc.hpp>
#include <planet/serialise/muxing.hpp>


namespace planet::comms::inproc {


    /// ## Intra-process data queue
    /**
     * This type moves data buffers between threads within the same process. The
     * design is such that types with the same interface can be used for moving
     * data between processes, or between hosts over the network.
     *
     * Subscribers provide the box name that they wish to receive and will then
     * get all data that appears within that box.
     */
    class data final :
    public serialise::demuxer,
            private queue::tspsc<serialise::shared_bytes> {
        comms::signal signalling;

        telemetry::counter pushes{name() + "__pushes"},
                deliveries{name() + "__deliveries"};


      public:
        /// ### Construction
        /**
         * Pass the constructor the warden that is used by the thread which
         * consumes values.
         */
        data(std::string_view, felspar::io::warden &);


        /// ### Subscribe to a given box name
        queue::pmc<serialise::demuxer::message>::consumer
                subscribe(std::string_view);


      private:
        void do_push(serialise::shared_bytes b) override;
        felspar::io::warden::task<std::span<serialise::shared_bytes>>
                acquire() override;
    };


}
