#include <planet/comms/inproc.hpp>
#include <planet/comms/signal.hpp>
#include <planet/log.hpp>
#include <planet/queue/tspsc.hpp>
#include <planet/serialise/chrono.hpp>
#include <planet/serialise/load_buffer.hpp>
#include <planet/telemetry/counter.hpp>
#include <planet/telemetry/rate.hpp>
#include <planet/time/checkpointer.hpp>

#include <felspar/io/warden.poll.hpp>
#include <felspar/memory/hexdump.hpp>

#include <iostream>
#include <thread>


using namespace std::literals;


thread_local planet::serialise::save_buffer planet::log::detail::ab;


namespace {
    constexpr std::string_view log_root_directory = LOG_ROOT_DIRECTORY;


    auto g_start_time() {
        static auto st = std::chrono::steady_clock::now();
        return st;
    }
    [[maybe_unused]] auto const g_started = g_start_time();


    auto &printers_mutex() {
        static std::mutex m;
        return m;
    }
    auto &printers() {
        static std::map<std::string_view, planet::log::detail::formatter const *>
                m;
        return m;
    }


    void show(std::ostream &, planet::serialise::load_buffer &, std::size_t);
    void
            show(std::ostream &os,
                 planet::serialise::box &b,
                 std::size_t const depth) {
        if (auto printer =
                    [&]() {
                        std::scoped_lock _{printers_mutex()};
                        return printers().find(b.name);
                    }();
            printer == printers().end()) {
            os << "box " << b.name << " [v" << int(b.version) << " size "
               << b.content.size() << " bytes]\n"
               << std::string(depth + 1, ' ');
            show(os, b.content, depth + 1);
        } else {
            try {
                printer->second->print(os, b, depth);
            } catch (std::exception const &e) {
                os << "\n\nError formatting output for box '" << b.name
                   << "': " << e.what() << '\n';
            }
        }
    }
    void
            show1(std::ostream &os,
                  planet::serialise::load_buffer &lb,
                  std::size_t const depth) {
        auto const mv = static_cast<std::uint8_t>(lb.cmemory()[0]);
        if (mv > 0 and mv < 80) {
            auto b = expect_box(lb);
            show(os, b, depth);
        } else {
            auto const m = lb.extract_marker();
            switch (m) {
            case planet::serialise::marker::empty: os << "empty"; break;

            case planet::serialise::marker::std_byte_array: {
                auto const size = lb.extract_size_t();
                auto s = lb.split(size);
                felspar::memory::hexdump(os, s);
                break;
            }

            case planet::serialise::marker::b_true: os << "true"; break;
            case planet::serialise::marker::b_false: os << "false"; break;

            case planet::serialise::marker::i8:
                os << std::to_string(lb.extract<std::int8_t>());
                break;
            case planet::serialise::marker::u8:
                os << std::to_string(lb.extract<std::uint8_t>());
                break;
            case planet::serialise::marker::i16le:
                os << std::to_string(lb.extract<std::int16_t>());
                break;
            case planet::serialise::marker::u16le:
                os << std::to_string(lb.extract<std::uint16_t>());
                break;
            case planet::serialise::marker::i32le:
                os << std::to_string(lb.extract<std::int32_t>());
                break;
            case planet::serialise::marker::u32le:
                os << std::to_string(lb.extract<std::uint32_t>());
                break;
            case planet::serialise::marker::i64le:
                os << std::to_string(lb.extract<std::int64_t>());
                break;
            case planet::serialise::marker::u64le:
                os << std::to_string(lb.extract<std::uint64_t>());
                break;

            case planet::serialise::marker::f32le:
                os << lb.extract<float>();
                break;
            case planet::serialise::marker::f64le:
                os << lb.extract<double>();
                break;
            case planet::serialise::marker::f128le:
                os << lb.extract<long double>();
                break;

            case planet::serialise::marker::poly_list: {
                auto const count = lb.extract_size_t();
                os << "[poly-list with " << count << " items]";
                for (std::size_t index{}; index < count; ++index) {
                    os << '\n' << std::string(depth + 1, ' ');
                    show1(os, lb, depth + 1);
                }
                os << '\n' << std::string(depth, ' ');
                break;
            }

            case planet::serialise::marker::u8string8: {
                auto const buffer = lb.split(lb.extract_size_t());
                os << std::string_view{
                        reinterpret_cast<char const *>(buffer.data()),
                        buffer.size()};
                break;
            }

            default:
                os << "unknown marker [" << to_string(m) << " - 0x" << std::hex
                   << static_cast<unsigned>(m) << std::dec << ']';
                return;
            }
        }
    }
    void
            show(std::ostream &os,
                 planet::serialise::load_buffer &lb,
                 std::size_t const depth) {
        while (not lb.empty()) {
            show1(os, lb, depth);
            if (depth == 0) {
                os << ' ';
            } else {
                os << '\n' << std::string(depth, ' ');
            }
        }
    }


    void print(planet::log::message const &m) {
        std::cout << std::fixed
                  << static_cast<double>(
                             (m.logged - g_start_time()).count() / 1e9)
                  << std::defaultfloat << ' ';
        switch (m.level) {
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
            std::cout << "\33[0;31mCRITICAL ERROR\33[0;39m ";
            break;
        }
        planet::serialise::load_buffer buffer{m.payload.cmemory()};
        show(std::cout, buffer, 0);
        std::string_view fn{m.location.file_name()};
        if (not log_root_directory.empty()
            and fn.starts_with(log_root_directory)) {
            fn.remove_prefix(log_root_directory.size() + 1);
        }
        std::cout << " \33[0;37m" << m.location.function_name() << ' ' << fn
                  << ':' << m.location.line() << ':' << m.location.column()
                  << "\33[0;39m" << std::endl;
    }


    planet::telemetry::counter debug_count{"planet_log_message_debug"};
    planet::telemetry::counter info_count{"planet_log_message_info"};
    planet::telemetry::counter warning_count{"planet_log_message_warning"};
    planet::telemetry::counter error_count{"planet_log_message_error"};


    struct log_thread {
        felspar::io::poll_warden warden;
        planet::comms::inproc::
                object<planet::log::message, planet::serialise::shared_bytes>
                        bus{"planet_log_bus", warden,
                            planet::telemetry::id::suffix::suppress};
        planet::comms::signal terminate{warden};

        log_thread() {}
        ~log_thread() {
            if (thread.joinable()) { stop_thread(); }
        }
        void stop_thread() {
            terminate.send({});
            thread.join();
        }
        std::thread thread{[this]() {
            try {
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
            tasks.post(
                    *this,
                    &log_thread::display_logged_performance_counters_loop);
            tasks.post(*this, &log_thread::display_log_messages_loop);
            std::array<std::byte, 1> buffer;
            co_await terminate.read_some(buffer);
        }

        void print_lgc(
                std::chrono::steady_clock::time_point const &logged,
                std::span<std::byte const> const payload) {
            std::cout << "\33[0;32m" << std::fixed
                      << static_cast<double>(
                                 (logged - g_start_time()).count() / 1e9)
                      << std::defaultfloat
                      << " Performance counters\33[0;39m\n  ";
            planet::serialise::load_buffer lb{payload};
            show(std::cout, lb, 2);
            std::cout << std::endl;
        }
        void print_performance() {
            planet::telemetry::performance::current_values(
                    planet::log::detail::ab);
            auto const bytes = planet::log::detail::ab.complete();
            planet::log::logged_performance_counters lgc{.counters = bytes};
            if (planet::log::display_performance_messages.load()) {
                print_lgc(lgc.logged, lgc.counters.cmemory());
            }
            if (auto *out = planet::log::log_output.load(); out) {
                save(planet::log::detail::ab, lgc);
                auto const log_data = planet::log::detail::ab.complete();
                (*out).write(
                        reinterpret_cast<char const *>(log_data.data()),
                        log_data.size());
            }
        }
        felspar::io::warden::task<void> display_performance_loop() {
            planet::log::info("Starting performance counter loop");
            while (true) {
                co_await warden.sleep(1s);
                print_performance();
            }
        }
        felspar::io::warden::task<void>
                display_logged_performance_counters_loop() {
            while (true) {
                auto &lpcs = bus.queue_for<planet::serialise::shared_bytes>();
                while (true) {
                    auto bytes = co_await lpcs.next();
                    planet::serialise::load_buffer lb{bytes};
                    try {
                        auto box = planet::serialise::expect_box(lb);
                        std::chrono::steady_clock::time_point logged;
                        std::span<std::byte const> counters;
                        box.named(
                                planet::log::logged_performance_counters::box,
                                logged, counters);
                        print_lgc(logged, counters);
                    } catch (std::exception const &err) {
                        planet::log::error(
                                "Showing logged performance counters error:",
                                err.what());
                    }
                }
            }
        }


        felspar::io::warden::task<void> display_log_messages_loop() {
            std::cout << std::setprecision(9);
            auto &messages = bus.queue_for<planet::log::message>();
            while (true) {
                auto message = co_await messages.next();
                auto out = planet::log::log_output.load();
                if (out) {
                    save(planet::log::detail::ab, message);
                    auto const bytes = planet::log::detail::ab.complete();
                    out->write(
                            reinterpret_cast<char const *>(bytes.data()),
                            bytes.size());
                    out->flush();
                }
                print(message);
                switch (message.level) {
                case planet::log::level::debug: ++debug_count; break;
                case planet::log::level::info: ++info_count; break;
                case planet::log::level::warning: ++warning_count; break;
                case planet::log::level::error: ++error_count; break;
                case planet::log::level::critical:
                    /**
                     * TODO We should probably print the rest of the log
                     * messages we have before we exit the game.
                     */
                    std::cout
                            << "\33[0;31mA Critical log message is forcing an unclean shutdown\33[0;39m"
                            << std::endl;
                    print_performance();
                    if (auto *flushing = planet::log::log_output.load();
                        flushing) {
                        flushing->flush();
                    }
                    std::exit(120);
                }
            }
        }
    };

    auto &g_log_thread() {
        static log_thread lt;
        return lt;
    }
}


void planet::log::stop_thread() {
    auto &thread = g_log_thread();
    thread.stop_thread();
    thread.print_performance();
}


void planet::log::detail::critical_log_encountered() {
    /**
     * Wait for a bit here.
     *
     * The terminate that is actually meaningful is the one a few lines above
     * which will cause the program to terminate after dealing with the log
     * message this is called from. The one here is just to ensure that this
     * function doesn't actually return.
     */
    std::this_thread::sleep_for(2s);
    std::exit(121);
}


namespace {
    planet::telemetry::counter message_count{"planet_log_message_count"};
    planet::telemetry::real_time_rate message_rate{
            "planet_log_message_rate", 2s};
}
void planet::log::detail::write_log(
        level const l,
        serialise::shared_bytes b,
        std::source_location const &loc) {
    auto &lt = g_log_thread();
    lt.bus.push(planet::log::message{l, std::move(b), loc});
    ++message_count;
    message_rate.tick();
}


void planet::log::pretty_print(
        std::ostream &os, serialise::load_buffer &lb, std::size_t const depth) {
    show1(os, lb, depth);
}
void planet::log::pretty_print(
        std::ostream &os, serialise::box &b, std::size_t const depth) {
    show(os, b, depth);
}


/// ## `planet::log::detail::formatter`


planet::log::detail::formatter::formatter(std::string_view const n)
: box_name{n} {
    std::scoped_lock _{printers_mutex()};
    printers()[box_name] = this;
}


planet::log::detail::formatter::~formatter() {
    /**
     * Because we don't control the destruction order it's possible for a
     * formatter to be destroyed after the global in this translation unit has
     * gone. Because these are all static functions anyway it's fine to just let
     * this "leak".
     *
     * TODO The way to fix this is to have the formatter hold a shared pointer
     * to the printers and the mutex. That way the last formatter to be
     * destroyed gets rid of the printers.
     */
    // std::scoped_lock _{printers_mutex()};
    // printers().erase(printers().find(box_name));
}


namespace {
    auto const std_optional = planet::log::format(
            "_s:opt",
            [](std::ostream &os,
               planet::serialise::box &box,
               std::size_t const depth) {
                auto const has_value =
                        planet::serialise::load_type<bool>(box.content);
                if (has_value) {
                    os << "[std::optional] ";
                    show1(os, box.content, depth + 1);
                } else {
                    os << "[empty std::optional]";
                }
            });
    auto const std_set = planet::log::format(
            "_s:set",
            [](std::ostream &os,
               planet::serialise::box &box,
               std::size_t const depth) {
                std::size_t const count =
                        planet::serialise::load_type<std::size_t>(box.content);
                if (count == 0) {
                    os << "[empty std::set]";
                } else {
                    os << "[std::set with " << count << " items]";
                    for (std::size_t index{}; index < count; ++index) {
                        os << '\n' << std::string(depth + 1, ' ');
                        show1(os, box.content, depth + 1);
                    }
                    os << '\n' << std::string(depth, ' ');
                }
            });
}


/// ## `planet::log::counters`


auto planet::log::counters::current() noexcept -> counters {
    return {debug_count.value(), info_count.value(), warning_count.value(),
            error_count.value()};
}


/// ## `planet::log::file_header`


void planet::log::write_file_headers() {
    write_file_headers(detail::ab);
    auto const bytes = detail::ab.complete();
    if (auto *out = planet::log::log_output.load(); out) {
        (*out).write(
                reinterpret_cast<char const *>(bytes.data()), bytes.size());
    }
}
void planet::log::write_file_headers(serialise::save_buffer &sb) {
    sb.save_box(file_header::box, g_start_time(), log_root_directory);
}
void planet::log::load_fields(serialise::box &b, file_header &f) {
    b.fields(f.base_time, f.file_prefix);
    b.check_empty_or_throw();
}


/// ## `planet::log::logged_performance_counters`


void planet::log::save(
        serialise::save_buffer &ab, logged_performance_counters const &l) {
    ab.save_box(l.box, l.logged, l.counters);
}


void planet::log::logged_performance_counters::print(
        serialise::shared_bytes const d) {
    g_log_thread().bus.push(d);
}


/// ## `planet::log::message`


void planet::log::save(serialise::save_buffer &ab, message const m) {
    ab.save_box(m.box, m.level, m.location, m.logged, m.payload);
}
