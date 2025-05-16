#pragma once


#include <planet/comms/signal.hpp>
#include <planet/queue/psc.hpp>
#include <planet/queue/tspsc.hpp>
#include <planet/serialise/muxing.hpp>

#include <variant>


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
    class data final : public serialise::demuxer {
        comms::signal signalling;
        queue::tspsc<serialise::shared_bytes> queue;

        telemetry::counter pushes{name() + "__pushes"},
                deliveries{name() + "__deliveries"};


      public:
        /// ### Construction
        /**
         * Pass the constructor the warden that is used by the thread which
         * consumes values.
         */
        data(std::string_view, felspar::io::warden &);


        /// ### Push data
        using serialise::demuxer::push;


        /// ### Subscribe to a given box name
        queue::pmc<serialise::demuxer::message>::consumer
                subscribe(std::string_view);


      private:
        void do_push(serialise::shared_bytes b) override;
        felspar::io::warden::task<std::span<serialise::shared_bytes>>
                acquire() override;
    };


    /// ## Intra-process object queue
    /**
     * Values can be sent from any thread. Subscribers provide the type they are
     * interested in receiving.
     *
     * Items of any of the types in the queue can be pushed from any thread, but
     * all subscribers must run in the same thread.
     *
     * Because this type is moving objects and not serialised data between
     * threads, there can be no inter-process or between host version of this
     * API.
     */
    template<typename... Types>
    class object final :
    public telemetry::id,
            private queue::tspsc<std::variant<Types...>> {
        using tspsc = queue::tspsc<std::variant<Types...>>;
        using buses_type = std::tuple<queue::psc<Types>...>;


        comms::signal signalling;
        buses_type buses;

        telemetry::counter sends{name() + "__sends"},
                deliveries{name() + "__deliveries"};


      public:
        using variant_type = std::variant<Types...>;


        /// ### Construction
        /**
         * Pass the constructor the warden for the thread that the subscribers
         * will be running in.
         */
        object(std::string_view const n,
               felspar::io::warden &w,
               telemetry::id::suffix const s = telemetry::id::suffix::yes)
        : id{n, s}, signalling{w} {}

        object(object const &) = delete;
        object(object &&) = delete;
        object &operator=(object const &) = delete;
        object &operator=(object &&) = delete;


        /// ### Send an object to receiving thread
        template<typename T>
        void push(T t) {
            tspsc::push(variant_type{std::move(t)});
            signalling.send({});
            ++sends;
        }


        /// ### Buses for the various types
        template<typename T>
        auto &queue_for() {
            if (manager.empty()) {
                manager.post(object::manage_subscriptions());
            }
            return std::get<queue::psc<T>>(buses);
        }


      private:
        felspar::io::warden::eager<> manager;
        felspar::io::warden::eager<>::task_type manage_subscriptions() {
            while (true) {
                auto processing = co_await acquire();
                for (auto &v : processing) {
                    std::visit(
                            [this](auto &&t) {
                                using type =
                                        std::remove_reference_t<decltype(t)>;
                                std::get<queue::psc<type>>(this->buses)
                                        .push(std::move(t));
                                ++this->deliveries;
                            },
                            std::move(v));
                }
            }
        }

        felspar::io::warden::task<std::span<variant_type>> acquire() {
            auto s = tspsc::consume();
            while (s.empty()) {
                std::array<std::byte, 16> buffer;
                co_await signalling.read_some(buffer);
                s = tspsc::consume();
            }
            co_return s;
        }
    };


}
