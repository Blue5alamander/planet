#include <planet/asset_manager.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("asset_manager");


    auto const cwd = std::filesystem::current_path();


    auto const fn = suite.test("paths/filename", [](auto check) {
        planet::asset_manager am{"./filename"};
        auto gen = am.search_paths();
        check(gen.next().value()) == cwd / "var/";
        check(gen.next().has_value()) == false;
    });
    auto const pn = suite.test("paths/pathname", [](auto check) {
        planet::asset_manager am{"./path/filename"};
        auto gen = am.search_paths();
        check(gen.next().value()) == cwd / "var/";
        check(gen.next().value()) == cwd / "path/var/";
        check(gen.next().has_value()) == false;
    });
    auto const rn = suite.test("paths/rootname", [](auto check) {
        planet::asset_manager am{"/path/filename"};
        auto gen = am.search_paths();
        check(gen.next().value()) == cwd / "var/";
        check(gen.next().value()) == std::filesystem::path{"/path/var/"};
        check(gen.next().has_value()) == false;
    });


}
