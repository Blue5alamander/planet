#include <planet/asset_manager.hpp>

#include <iostream>


int main(int const argc, char const *const argv[]) {
    std::cout << ".ogg file player\n";
    if (argc == 2) {
        std::cout << "Loading " << argv[1] << '\n';
        auto const ogg = planet::file_loader::file_data(argv[1]);
        return 0;
    } else {
        std::cerr << "Usage\n\t" << argv[0] << " music.ogg\n";
        return 1;
    }
}
