#include <planet/ui/layout.grid.hpp>
#include <planet/debug/ui.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("grid.layout");


    auto const rows1 = suite.test("one row", [](auto check, auto &log) {
        auto r2 = planet::ui::grid{
                std::array{
                        planet::debug::ui_element{{3, 4}},
                        planet::debug::ui_element{{3, 4}}},
                {}};

        check(r2.extents({10, 10}).width) == 6.0f;
        check(r2.extents({10, 10}).height) == 4.0f;

        r2.hpadding = 2.0f;
        r2.vpadding = 3.0f;

        check(r2.extents({10, 10}).width) == 8.0f;
        check(r2.extents({10, 10}).height) == 4.0f;
    });


}
