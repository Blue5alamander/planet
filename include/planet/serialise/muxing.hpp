#pragma once


#include <planet/queue/pmc.hpp>
#include <planet/serialise/load_buffer.hpp>
#include <planet/telemetry/counter.hpp>
#include <planet/telemetry/id.hpp>

#include <felspar/coro/eager.hpp>
#include <felspar/io/warden.hpp>

#include <map>


namespace planet::serialise {


    /// ## Demuxing of data
    class demuxer : public telemetry::id {
      protected:
        /// ### Implemented by the sub-class

        /// #### Add the data to the queue
        virtual void push(shared_bytes) = 0;

        /// #### Acquire the data
        virtual felspar::io::warden::task<std::span<shared_bytes>> acquire() = 0;

        /// Called by the sub-class once the required virtuals are set up
        void start_manager();


      public:
        demuxer();
        demuxer(std::string_view);


        /// ### Message envolope
        struct message {
            serialise::box box;
            shared_bytes keep_alive;

            /// #### Helper for loading a type out of the message
            template<typename T>
            T load_type() {
                return serialise::load_type<T>(box);
            }
        };


        /// ### Return the queue for a given box name
        queue::pmc<message> &queue_for(std::string_view);


        /// ### Send/receive data to the subscribers
        void send(shared_bytes);


      private:
        /// ### Manage subscriptions
        /// TODO There's probably a better data structure for this
        std::map<std::string, queue::pmc<message>, std::less<>> subscribers;
        felspar::io::warden::eager<> manager;
        felspar::io::warden::task<void> manage_simulation_subscriptions();


        telemetry::counter sends{name() + "__sends"},
                enqueued{name() + "__enqueued"};
    };


    /// ## Load an instance of a type from a demuxer message
    /// Or use the member function in the `demuxer::message`
    template<typename T>
    T load_type(demuxer::message m) {
        return serialise::load_type<T>(m.box);
    }


}
