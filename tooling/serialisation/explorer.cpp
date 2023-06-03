#include <planet/serialise.hpp>

#include <felspar/parse.binary.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>


using namespace felspar::parse::binary;
using namespace planet;


namespace {
    void show(serialise::load_buffer &lb, std::size_t const depth) {
        while (not lb.empty()) {
            std::cout << std::string(depth, ' ');

            auto const mv = static_cast<std::uint8_t>(lb.cmemory()[0]);
            if (mv > 0 and mv < 80) {
                auto b = load_type<serialise::box>(lb);
                std::cout << b.name << " v" << int(b.version) << " size "
                          << b.content.size() << " bytes\n";
                show(b.content, depth + 1);
            } else {
                auto const m = lb.extract_marker();
                switch (m) {
                case serialise::marker::empty: std::cout << "empty\n"; break;

                case serialise::marker::b_true: std::cout << "true\n"; break;
                case serialise::marker::b_false: std::cout << "false\n"; break;

                case serialise::marker::i32le:
                    std::cout << lb.extract<std::int32_t>() << " (LE)\n";
                    break;
                case serialise::marker::u64le:
                    std::cout << lb.extract<std::uint64_t>() << " (LE)\n";
                    break;
                case serialise::marker::i64le:
                    std::cout << lb.extract<std::int64_t>() << " (LE)\n";
                    break;

                case serialise::marker::f128le:
                    std::cout << lb.extract<long double>() << " (LE)\n";
                    break;

                case serialise::marker::poly_list: {
                    auto const count = lb.extract_size_t();
                    std::cout << "poly-list with " << count << " items\n";
                    for (std::size_t index{}; index < count; ++index) {
                        show(lb, depth + 1);
                    }
                    break;
                }

                default:
                    std::cerr << "unknown marker " << to_string(m) << " - 0x" << std::hex
                              << static_cast<unsigned>(m) << std::dec << '\n';
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
    std::vector<char> save(std::filesystem::file_size(fn));
    input.read(save.data(), save.size());

    serialise::load_buffer buffer{
            std::as_bytes(std::span{save.data(), save.size()})};
    show(buffer, 0);

    return 0;
}
