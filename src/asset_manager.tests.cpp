#include <planet/asset_manager.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("file_loader");
    auto const cwd = std::filesystem::current_path();
    auto const fn = suite.test("paths/filename", [](auto check) {
        planet::file_loader am{"./"};
        auto gen = am.search_paths();
        check(gen.next().value()) == cwd / "share/";
        check(gen.next().has_value()) == false;
    });
    auto const pn = suite.test("paths/pathname", [](auto check) {
        planet::file_loader am{"./path"};
        auto gen = am.search_paths();
        check(gen.next().value()) == cwd / "share/";
        check(gen.next().value()) == cwd / "path/share/";
        check(gen.next().has_value()) == false;
    });
    auto const rn = suite.test("paths/rootname", [](auto check) {
        planet::file_loader am{"/path"};
        auto gen = am.search_paths();
        check(gen.next().value()) == cwd / "share/";
#ifdef _WIN32
        /**
         * The drive is whatever the test runs from: C: on a normal Windows
         * host, but Z: under wine (which maps the Unix root there). Derive it
         * from the cwd instead of hard-coding C: so the test passes in both.
         */
        check(gen.next().value()) == cwd.root_path() / "path/share/";
        check(gen.next().value()) == cwd.root_path() / "share/";
#else
        check(gen.next().value()) == std::filesystem::path{"/path/share/"};
        check(gen.next().value()) == std::filesystem::path{"/share/"};
#endif
        check(gen.next().has_value()) == false;
    });


}
