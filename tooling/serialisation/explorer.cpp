#include <planet/serialise.hpp>

#include <felspar/parse.binary.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>


using namespace felspar::parse::binary;
using namespace planet;


namespace {
    void show(serialise::load_buffer &lb, std::size_t const depth) {
        if (lb.empty()) { return; }
        std::cout << std::string(depth, ' ');
        auto const mv = static_cast<std::uint8_t>(lb.cmemory()[0]);
        if (mv > 0 and mv < 80) {
            auto b = load_type<serialise::box>(lb);
            std::cout << b.name << " v" << int(b.version) << " size "
                      << b.content.size() << " bytes\n";
            show(b.content, depth + 1);
            show(lb, depth);
        } else {
            auto const m = serialise::marker{mv};
            switch (m) {
            case serialise::marker::empty: std::cout << "empty\n"; break;

            default:
                std::cerr << "unknown marker " << static_cast<unsigned>(m)
                          << '\n';
                return;
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
