#include <planet/ui/box.hpp>
#include <felspar/test.hpp>


namespace {


    constexpr planet::affine::extents2d unit{1, 1};
    constexpr planet::affine::rectangle2d unitbox{{0, 0}, unit};
    constexpr planet::affine::rectangle2d testbox{
            {2, 3}, planet::affine::point2d{6, 8}};

    auto const gh = felspar::testsuite(
            "gravity/horizontal",
            [](auto check) {
                auto const v = planet::ui::within({}, unitbox, unit);
                check(v.left()) == 0;
                check(v.top()) == 0;
                check(v.right()) == 1;
                check(v.bottom()) == 1;
            },
            [](auto check) {
                auto const v = planet::ui::within({}, testbox, unit);
                check(v.left()) == 2;
                check(v.top()) == 3;
                check(v.right()) == 6;
                check(v.bottom()) == 8;
            },
            [](auto check) {
                auto const v = planet::ui::within(
                        planet::ui::gravity::left, testbox, unit);
                check(v.left()) == 2;
                check(v.top()) == 3;
                check(v.right()) == 3;
                check(v.bottom()) == 8;
            },
            [](auto check) {
                auto const v = planet::ui::within(
                        planet::ui::gravity::right, testbox, unit);
                check(v.left()) == 5;
                check(v.top()) == 3;
                check(v.right()) == 6;
                check(v.bottom()) == 8;
            },
            [](auto check) {
                auto const v = planet::ui::within(
                        planet::ui::gravity::right | planet::ui::gravity::left,
                        testbox, unit);
                check(v.left()) == 3.5f;
                check(v.top()) == 3;
                check(v.right()) == 4.5f;
                check(v.bottom()) == 8;
            },
            [](auto check) {
                auto const v = planet::ui::within(
                        planet::ui::gravity::right | planet::ui::gravity::left,
                        unitbox, {4, 5});
                check(v.left()) == -1.5f;
                check(v.top()) == 0;
                check(v.right()) == 2.5f;
                check(v.bottom()) == 1;
            });

    auto const gv = felspar::testsuite(
            "gravity/vertical",
            [](auto check) {
                auto const v = planet::ui::within(
                        planet::ui::gravity::top, testbox, unit);
                check(v.left()) == 2;
                check(v.top()) == 3;
                check(v.right()) == 6;
                check(v.bottom()) == 4;
            },
            [](auto check) {
                auto const v = planet::ui::within(
                        planet::ui::gravity::bottom, testbox, unit);
                check(v.left()) == 2;
                check(v.top()) == 7;
                check(v.right()) == 6;
                check(v.bottom()) == 8;
            },
            [](auto check) {
                auto const v = planet::ui::within(
                        planet::ui::gravity::top | planet::ui::gravity::bottom,
                        testbox, unit);
                check(v.left()) == 2;
                check(v.top()) == 5;
                check(v.right()) == 6;
                check(v.bottom()) == 6;
            },
            [](auto check) {
                auto const v = planet::ui::within(
                        planet::ui::gravity::top | planet::ui::gravity::bottom,
                        unitbox, {4, 5});
                check(v.left()) == 0;
                check(v.top()) == -2;
                check(v.right()) == 1;
                check(v.bottom()) == 3;
            },
            [](auto check) {
                auto const v = planet::ui::within(
                        planet::ui::gravity::top | planet::ui::gravity::bottom
                                | planet::ui::gravity::right
                                | planet::ui::gravity::left,
                        testbox, unit);
                check(v.left()) == 3.5f;
                check(v.top()) == 5;
                check(v.right()) == 4.5f;
                check(v.bottom()) == 6;
            },
            [](auto check) {
                auto const v = planet::ui::within(
                        planet::ui::gravity::top | planet::ui::gravity::bottom
                                | planet::ui::gravity::right
                                | planet::ui::gravity::left,
                        unitbox, {4, 5});
                check(v.left()) == -1.5f;
                check(v.top()) == -2;
                check(v.right()) == 2.5f;
                check(v.bottom()) == 3;
            });


}
