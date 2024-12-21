#include <planet/serialise.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>


namespace {
    void show(planet::serialise::load_buffer &, std::size_t);
    void show(planet::serialise::box &box, std::size_t const depth) {
        std::cout << box.name << " v" << int(box.version) << " size "
                  << box.content.size() << " bytes";
        if (box.name == "_s:opt") {
            auto const has_value = load_type<bool>(box.content);
            if (has_value) {
                std::cout << " (with value)\n";
                show(box.content, depth + 1);
            } else if (not box.content.empty()) {
                std::cout << " (shows as empty, but has content)\n";
                show(box.content, depth + 1);
            } else {
                std::cout << " (empty)\n";
            }
        } else if (box.name == "_s:map") {
            auto const entries = [&]() {
                if (box.version == 2) {
                    return load_type<std::size_t>(box.content);
                } else if (box.version == 1) {
                    return box.content.extract_size_t();
                } else {
                    box.throw_unsupported_version(2);
                }
            }();
            std::cout << " with " << entries << " entries\n";
            show(box.content, depth + 1);
        } else {
            std::cout << '\n';
            show(box.content, depth + 1);
        }
    }
    void show(planet::serialise::load_buffer &lb, std::size_t const depth) {
        while (not lb.empty()) {
            std::cout << std::string(depth, ' ');

            auto const mv = static_cast<std::uint8_t>(lb.cmemory()[0]);
            if (mv > 0 and mv < 80) {
                auto box = expect_box(lb);
                show(box, depth);
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

                case planet::serialise::marker::u8:
                    std::cout
                            << static_cast<unsigned>(lb.extract<std::uint8_t>())
                            << ' ' << to_string(m) << '\n';
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

                case planet::serialise::marker::u8string8: {
                    std::string str;
                    auto const sz = lb.extract_size_t();
                    auto const b = lb.split(sz);
                    str = {reinterpret_cast<char const *>(b.data()), b.size()};
                    std::cout << str << '\n';
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


int main(int argc, char const *argv[]) {
    if (argc < 2) {
        std::cout << argv[0] << " filename\n";
        return 1;
    }

    std::filesystem::path const fn{argv[1]};
    std::ifstream input{fn};
    std::vector<char> save(std::filesystem::file_size(fn), std::ios::binary);
    input.read(save.data(), save.size());

    planet::serialise::load_buffer buffer{
            std::as_bytes(std::span{save.data(), save.size()})};
    show(buffer, 0);

    return 0;
}
