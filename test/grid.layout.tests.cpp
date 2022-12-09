#include <planet/ui/layout.grid.hpp>
#include <planet/debug/ui.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("grid.layout");


    auto const rows1a = suite.test("one row/array", [](auto check, auto &log) {
        auto r1 = planet::ui::grid{
                std::array{planet::debug::ui_element{{3, 4}}}, {}};
        check(r1.extents({10, 10}).width) == 3.0f;
        check(r1.extents({10, 10}).height) == 4.0f;


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


        auto r3 = planet::ui::grid{
                std::array{
                        planet::debug::ui_element{{3, 4}},
                        planet::debug::ui_element{{3, 4}},
                        planet::debug::ui_element{{3, 4}}},
                {}};
        check(r3.extents({10, 10}).width) == 9.0f;
        check(r3.extents({10, 10}).height) == 4.0f;

        r3.hpadding = 1.0f;
        r3.vpadding = 3.0f;
        check(r3.extents({11, 10}).width) == 11.0f;
        check(r3.extents({11, 10}).height) == 4.0f;

        check(r3.extents({10, 10}).width) == 7.0f;
        check(r3.extents({10, 10}).height) == 11.0f;
    });


    auto const rows1t = suite.test("one row/tuple", [](auto check, auto &log) {
        auto r1 = planet::ui::grid{
                std::tuple{planet::debug::ui_element{{3, 4}}}, {}};
        check(r1.extents({10, 10}).width) == 3.0f;
        check(r1.extents({10, 10}).height) == 4.0f;


        auto r2 = planet::ui::grid{
                std::tuple{
                        planet::debug::ui_element{{3, 4}},
                        planet::debug::ui_element{{3, 4}}},
                {}};
        check(r2.extents({10, 10}).width) == 6.0f;
        check(r2.extents({10, 10}).height) == 4.0f;

        r2.hpadding = 2.0f;
        r2.vpadding = 3.0f;
        check(r2.extents({10, 10}).width) == 8.0f;
        check(r2.extents({10, 10}).height) == 4.0f;


        auto r3 = planet::ui::grid{
                std::tuple{
                        planet::debug::ui_element{{3, 4}},
                        planet::debug::ui_element{{3, 4}},
                        planet::debug::ui_element{{3, 4}}},
                {}};
        check(r3.extents({10, 10}).width) == 9.0f;
        check(r3.extents({10, 10}).height) == 4.0f;

        r3.hpadding = 1.0f;
        r3.vpadding = 3.0f;
        check(r3.extents({11, 10}).width) == 11.0f;
        check(r3.extents({11, 10}).height) == 4.0f;

        check(r3.extents({10, 10}).width) == 7.0f;
        check(r3.extents({10, 10}).height) == 11.0f;
    });


}
