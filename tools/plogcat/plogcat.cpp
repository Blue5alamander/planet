#include <planet/asset_manager.hpp>
#include <planet/log.hpp>

#include <felspar/memory/hexdump.hpp>

#include <iostream>


int main(int argc, char const *argv[]) {
    std::cout << "Planet binary log cat\n";
    if (argc != 2) {
        std::cerr << "Expected log file to display\n";
        return 1;
    } else {
        auto const filedata = planet::file_loader::file_data(argv[1]);
        std::cout << felspar::memory::hexdump(filedata);
        return 0;
    }
}
