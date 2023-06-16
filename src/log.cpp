#include <planet/comms/internal.hpp>
#include <planet/log.hpp>
#include <planet/queue/mpsc.hpp>
#include <planet/serialise/chrono.hpp>
#include <planet/serialise/load_buffer.hpp>

#include <felspar/io/warden.poll.hpp>

#include <iostream>
#include <thread>


namespace {
    auto g_start_time() {
        static auto st = std::chrono::steady_clock::now();
        return st;
    }

    struct message {
        planet::log::level level;
        planet::serialise::shared_bytes payload;
        std::chrono::steady_clock::time_point logged =
                std::chrono::steady_clock::now();

        void print() const;
    };

    struct log_thread {
        felspar::io::poll_warden warden;
        planet::queue::mpsc<message> messages;
        planet::comms::internal signal{warden};

        std::thread thread{[this]() {
            try {
                std::cout << "Starting logging thread" << std::endl;
                {
                    planet::serialise::save_buffer ab;
                    save(ab, g_start_time());
                    messages.push({planet::log::level::info, ab.complete()});
                    signal.write({});
                }
                warden.run(
                        +[](felspar::io::warden &, log_thread *ltp)
                                -> felspar::io::warden::task<void> {
                            auto &lt = *ltp;
                            while (true) {
                                auto messages = lt.messages.consume();
                                if (messages.empty()) {
                                    std::array<std::byte, 16> buffer;
                                    std::cout << "Waiting for log message"
                                              << std::endl;
                                    co_await lt.signal.read_some(buffer);
                                } else {
                                    std::cout << "Got " << messages.size()
                                              << " messages" << std::endl;
                                    for (auto const &message : messages) {
                                        message.print();
                                    }
                                }
                            }
                        },
                        this);
            } catch (...) { std::terminate(); }
        }};
    };

    auto &g_log_thread() {
        static log_thread lt;
        return lt;
    }
}


void planet::log::detail::write_log(level const l, serialise::shared_bytes b) {
    auto &lt = g_log_thread();
    lt.messages.push({l, std::move(b)});
    lt.signal.write({});
}


namespace {
    void show(planet::serialise::load_buffer &lb, std::size_t const depth) {
        while (not lb.empty()) {
            std::cout << std::string(depth, ' ');

            auto const mv = static_cast<std::uint8_t>(lb.cmemory()[0]);
            if (mv > 0 and mv < 80) {
                auto b = load_type<planet::serialise::box>(lb);
                std::cout << b.name << " v" << int(b.version) << " size "
                          << b.content.size() << " bytes\n";
                show(b.content, depth + 1);
            } else {
                auto const m = lb.extract_marker();
                switch (m) {
                case planet::serialise::marker::empty:
                    std::cout << "empty\n";
                    break;

                case planet::serialise::marker::b_true:
                    std::cout << "true\n";
                    break;
                case planet::serialise::marker::b_false:
                    std::cout << "false\n";
                    break;

                case planet::serialise::marker::i32le:
                    std::cout << lb.extract<std::int32_t>() << ' '
                              << to_string(m) << '\n';
                    break;
                case planet::serialise::marker::u64le:
                    std::cout << lb.extract<std::uint64_t>() << ' '
                              << to_string(m) << '\n';
                    break;
                case planet::serialise::marker::i64le:
                    std::cout << lb.extract<std::int64_t>() << ' '
                              << to_string(m) << '\n';
                    break;

                case planet::serialise::marker::f128le:
                    std::cout << lb.extract<long double>() << ' '
                              << to_string(m) << '\n';
                    break;

                case planet::serialise::marker::poly_list: {
                    auto const count = lb.extract_size_t();
                    std::cout << "poly-list with " << count << " items\n";
                    for (std::size_t index{}; index < count; ++index) {
                        show(lb, depth + 1);
                    }
                    break;
                }

                default:
                    std::cerr << "unknown marker " << to_string(m) << " - 0x"
                              << std::hex << static_cast<unsigned>(m)
                              << std::dec << '\n';
                    return;
                }
            }
        }
    }
}
void message::print() const {
    std::cout << (logged - g_start_time()).count() << ' ';
    switch (level) {
    case planet::log::level::debug: std::cout << "DEBUG "; break;
    case planet::log::level::info: std::cout << "INFO "; break;
    case planet::log::level::warning: std::cout << "WARNING "; break;
    case planet::log::level::error: std::cout << "ERROR "; break;
    case planet::log::level::critical: std::cout << "CRITICAL "; break;
    }
    planet::serialise::load_buffer buffer{payload.cmemory()};
    show(buffer, 0);
    std::cout << std::endl;
}
