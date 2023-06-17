#include <planet/comms/signal.hpp>
#include <planet/log.hpp>
#include <planet/queue/mpsc.hpp>
#include <planet/serialise/chrono.hpp>
#include <planet/serialise/load_buffer.hpp>
#include <planet/telemetry/counter.hpp>
#include <planet/telemetry/rate.hpp>
#include <planet/time/checkpointer.hpp>

#include <felspar/io/warden.poll.hpp>

#include <iostream>
#include <thread>


using namespace std::literals;


namespace {
    auto g_start_time() {
        static auto st = std::chrono::steady_clock::now();
        return st;
    }
    [[maybe_unused]] auto const g_started = g_start_time();


    struct message {
        planet::log::level level;
        planet::serialise::shared_bytes payload;
        std::chrono::steady_clock::time_point logged =
                std::chrono::steady_clock::now();

        void print() const;
    };


    void show(planet::serialise::load_buffer &lb, std::size_t const depth) {
        if (depth) { std::cout << std::string(depth, ' '); }
        while (not lb.empty()) {
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
                    std::cout << "empty";
                    break;

                case planet::serialise::marker::b_true:
                    std::cout << "true";
                    break;
                case planet::serialise::marker::b_false:
                    std::cout << "false";
                    break;

                case planet::serialise::marker::i32le:
                    std::cout << lb.extract<std::int32_t>();
                    break;
                case planet::serialise::marker::u64le:
                    std::cout << lb.extract<std::uint64_t>();
                    break;
                case planet::serialise::marker::i64le:
                    std::cout << lb.extract<std::int64_t>();
                    break;

                case planet::serialise::marker::f32le:
                    std::cout << lb.extract<float>();
                    break;
                case planet::serialise::marker::f128le:
                    std::cout << lb.extract<long double>();
                    break;

                case planet::serialise::marker::poly_list: {
                    auto const count = lb.extract_size_t();
                    std::cout << "poly-list with " << count << " items\n";
                    for (std::size_t index{}; index < count; ++index) {
                        show(lb, depth + 1);
                    }
                    break;
                }

                case planet::serialise::marker::u8string8: {
                    auto const buffer = lb.split(lb.extract_size_t());
                    std::cout << std::string_view{
                            reinterpret_cast<char const *>(buffer.data()),
                            buffer.size()};
                    break;
                }

                default:
                    std::cerr << "unknown marker [" << to_string(m) << " - 0x"
                              << std::hex << static_cast<unsigned>(m)
                              << std::dec << ']';
                    return;
                }
            }
            if (not lb.empty()) { std::cout << ' '; }
        }
    }


    struct log_thread {
        felspar::io::poll_warden warden;
        planet::queue::mpsc<message> messages;
        planet::comms::signal signal{warden};

        std::thread thread{[this]() {
            try {
                {
                    planet::serialise::save_buffer ab;
                    save(ab, g_start_time());
                    messages.push({planet::log::level::info, ab.complete()});
                    signal.send({});
                }
                warden.run(
                        +[](felspar::io::warden &, log_thread *ltp)
                                -> felspar::io::warden::task<void> {
                            co_await ltp->run_loops();
                        },
                        this);
            } catch (...) { std::terminate(); }
        }};
        felspar::io::warden::task<void> run_loops() {
            felspar::io::warden::starter<> tasks;
            tasks.post(*this, &log_thread::display_performance_loop);
            tasks.post(*this, &log_thread::display_log_messages_loop);
            co_await tasks.wait_for_all();
        }

        felspar::io::warden::task<void> display_performance_loop() {
            planet::serialise::save_buffer ab;
            while (true) {
                co_await warden.sleep(1s);
                planet::telemetry::performance::current_values(ab);
                std::cout << "\33[0;32mPerformance counters\33[0;39m";
                auto const bytes = ab.complete();
                planet::serialise::load_buffer lb{bytes.cmemory()};
                show(lb, 0);
                std::cout << std::endl;
            }
        }
        felspar::io::warden::task<void> display_log_messages_loop() {
            while (true) {
                auto block = messages.consume();
                if (block.empty()) {
                    std::array<std::byte, 16> buffer;
                    co_await signal.read_some(buffer);
                } else {
                    for (auto const &message : block) {
                        message.print();
                        if (message.level == planet::log::level::critical) {
                            std::terminate();
                        }
                    }
                }
            }
        }
    };

    auto &g_log_thread() {
        static log_thread lt;
        return lt;
    }
}


namespace {
    planet::telemetry::counter message_count{"log_message_count"};
    planet::telemetry::real_time_rate message_rate{"log_message_rate", 2s};
}
void planet::log::detail::write_log(level const l, serialise::shared_bytes b) {
    auto &lt = g_log_thread();
    lt.messages.push({l, std::move(b)});
    lt.signal.send({});

    ++message_count;
    message_rate.tick();
}


void message::print() const {
    std::cout << static_cast<double>((logged - g_start_time()).count() / 1e9)
              << ' ';
    switch (level) {
    case planet::log::level::debug:
        std::cout << "\33[0;37mDEBUG\33[0;39m ";
        break;
    case planet::log::level::info:
        std::cout << "\33[0;32mINFO\33[0;39m ";
        break;
    case planet::log::level::warning:
        std::cout << "\33[1;33mWARNING\33[0;39m ";
        break;
    case planet::log::level::error:
        std::cout << "\33[0;31mERROR\33[0;39m ";
        break;
    case planet::log::level::critical:
        std::cout << "\33[0;31mCRITICAL\33[0;39m ";
        break;
    }
    planet::serialise::load_buffer buffer{payload.cmemory()};
    show(buffer, 0);
    std::cout << std::endl;
}
