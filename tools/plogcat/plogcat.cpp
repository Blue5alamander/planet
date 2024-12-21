#include <planet/asset_manager.hpp>
#include <planet/log.hpp>

#include <felspar/memory/hexdump.hpp>

#include <iostream>


using namespace std::literals;


namespace {
    void fetch_and_print_timestamp(
            planet::log::file_header const &header,
            planet::serialise::box &box) {
        std::chrono::steady_clock::time_point when;
        load(box.content, when);
        auto const ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                when - header.base_time);
        if (ns < 1us) {
            std::cout << ns.count() << "ns ";
        } else if (ns < 1ms) {
            std::cout << (ns.count() / 1000) << "µs ";
        } else if (ns < 10s) {
            std::cout << (ns.count() / 1000'000) << "ms ";
        } else {
            std::cout << std::fixed << (double(ns.count()) / 1'000'000'000)
                      << std::defaultfloat << "s ";
        }
    }
}


int main(int argc, char const *argv[]) {
    try {
        std::cout << std::setprecision(9);
        std::cout << "Planet binary log cat\n";
        if (argc != 2) {
            std::cerr << "Expected log file to display\n";
            return 1;
        } else {
            planet::log::file_header header;

            auto const filedata = planet::file_loader::file_data(argv[1]);
            std::cout << "Printing \"" << argv[1] << "\" which is "
                      << filedata.size() << " bytes\n";
            planet::serialise::load_buffer lb{filedata};

            while (lb.size()) {
                auto box = planet::serialise::expect_box(lb);
                if (box.name == "_p:log:h") {
                    planet::log::load_fields(box, header);
                } else if (box.name == "_p:log:m") {
                    planet::log::level level{};
                    load(box.content, level);
                    switch (level) {
                    case planet::log::level::debug:
                        std::cout << "\33[0;37mDBUG\33[0;39m ";
                        break;
                    case planet::log::level::info:
                        std::cout << "\33[0;32mINFO\33[0;39m ";
                        break;
                    case planet::log::level::warning:
                        std::cout << "\33[1;33mWARN\33[0;39m ";
                        break;
                    case planet::log::level::error:
                        std::cout << "\33[0;31mERRR\33[0;39m ";
                        break;
                    case planet::log::level::critical:
                        std::cout << "\33[0;31mCRIT\33[0;39m ";
                        break;
                    }

                    std::string file, function;
                    std::uint_least32_t line, column;
                    box.content.load_box("_s:sl", file, function, line, column);
                    fetch_and_print_timestamp(header, box);
                    std::cout << function << ' ' << file << ':' << line << ':'
                              << column << '\n';

                    std::span<std::byte const> payload;
                    load(box.content, payload);
                    planet::serialise::load_buffer lb{payload};
                    planet::log::pretty_print(lb);

                    std::cout << "\n\n";
                } else if (box.name == "_p:log:c") {
                    std::cout << "\33[0;32mPERF\33[0;39m ";

                    fetch_and_print_timestamp(header, box);
                    std::cout << '\n';

                    std::span<std::byte const> payload;
                    load(box.content, payload);
                    planet::serialise::load_buffer lb{payload};
                    while (lb.size()) {
                        auto counter = planet::serialise::expect_box(lb);
                        planet::log::pretty_print(counter);
                        std::cout << '\n';
                    }

                    std::cout << "\n\n";
                } else {
                    std::cout
                            << box.name << ": "
                            << felspar::memory::hexdump(box.content.cmemory());
                }
            }
            return 0;
        }
    } catch (std::exception const &e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 2;
    }
}
