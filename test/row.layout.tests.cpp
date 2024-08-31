#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/layout.row.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;

    constexpr planet::ui::reflowable::constrained_type screen{
            {100, 0, 100}, {100, 0, 100}};


    auto const rsuite = felspar::testsuite("row.layout");


    auto const arf1 = rsuite.test(
            "reflow/array",
            [](auto check, auto &log) {
                auto c = planet::ui::row{
                        std::array{planet::debug::fixed_element{log, {4, 3}}},
                        2};
                c.reflow({.screen = screen}, screen);
                c.move_to({{15, 20}, planet::affine::extents2d{100, 100}});

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
                check(c.items[0].position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
            },
            [](auto check, auto &log) {
                auto c = planet::ui::row{
                        std::array{
                                planet::debug::fixed_element{log, {4, 3}},
                                planet::debug::fixed_element{log, {4, 3}}},
                        2};
                c.reflow({.screen = screen}, screen);
                c.move_to({{15, 20}, planet::affine::extents2d{100, 100}});

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{10, 3}};
                check(c.items[0].position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
                check(c.items[1].position())
                        == planet::affine::rectangle2d{
                                {21, 20}, planet::affine::extents2d{4, 3}};
            });


    auto const rf1 = rsuite.test("reflow/tuple", [](auto check, auto &log) {
        auto c = planet::ui::row{
                std::tuple{planet::debug::fixed_element{log, {4, 3}}}, 2};
        c.reflow({.screen = screen}, screen);
        c.move_to({{15, 20}, planet::affine::extents2d{100, 100}});

        check(c.position())
                == planet::affine::rectangle2d{
                        {15, 20}, planet::affine::extents2d{4, 3}};
        check(std::get<0>(c.items).position())
                == planet::affine::rectangle2d{
                        {15, 20}, planet::affine::extents2d{4, 3}};
    });


}
