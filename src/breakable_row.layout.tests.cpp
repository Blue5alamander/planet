#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/layout.breakable_row.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;

    static constexpr planet::ui::reflowable::constrained_type screen{
            {100, 0, 100}, {100, 0, 100}};


    auto const bsuite = felspar::testsuite("breakable_row.layout");


    auto const arf = bsuite.test(
            "reflow/array",
            [](auto check, auto &log) {
                auto c = planet::ui::breakable_row{
                        std::array{planet::debug::fixed_element{log, {4, 3}}},
                        2};
                c.reflow({.screen = screen}, screen);
                c.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{100, 100}});

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
                check(c.items[0].position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
            },
            [](auto check, auto &log) {
                auto c = planet::ui::breakable_row{
                        std::array{
                                planet::debug::fixed_element{log, {4, 3}},
                                planet::debug::fixed_element{log, {4, 3}}},
                        2};
                c.reflow({.screen = screen}, screen);
                c.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{100, 100}});

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


    auto const trf = bsuite.test(
            "reflow/tuple",
            [](auto check, auto &log) {
                auto c = planet::ui::breakable_row{
                        std::tuple{planet::debug::fixed_element{log, {4, 3}}},
                        2};
                c.reflow({.screen = screen}, screen);
                c.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{100, 100}});

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
                check(std::get<0>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
            },
            [](auto check, auto &log) {
                auto c = planet::ui::breakable_row{
                        std::tuple{
                                planet::debug::fixed_element{log, {4, 3}},
                                planet::debug::fixed_element{log, {4, 3}}},
                        2};
                c.reflow({.screen = screen}, screen);
                c.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{100, 100}});

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{10, 3}};
                check(std::get<0>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
                check(std::get<1>(c.items).position())
                        == planet::affine::rectangle2d{
                                {21, 20}, planet::affine::extents2d{4, 3}};
            });


    /**
     * The row that wraps onto a second line must be separated from the first by
     * the vertical padding. The first row sits at `y == 0`, so a guard on `y`
     * used to swallow the padding for the very first wrap only.
     */
    auto const wrap =
            bsuite.test("reflow/array/wrap", [](auto check, auto &log) {
                auto c = planet::ui::breakable_row{
                        std::array{
                                planet::debug::fixed_element{log, {40, 3}},
                                planet::debug::fixed_element{log, {40, 3}},
                                planet::debug::fixed_element{log, {40, 3}}},
                        2, 5};
                c.reflow({.screen = screen}, screen);
                c.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{100, 100}});

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{82, 11}};
                check(c.items[0].position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{40, 3}};
                check(c.items[1].position())
                        == planet::affine::rectangle2d{
                                {57, 20}, planet::affine::extents2d{40, 3}};
                check(c.items[2].position())
                        == planet::affine::rectangle2d{
                                {15, 28}, planet::affine::extents2d{40, 3}};
            });


    auto const twrap =
            bsuite.test("reflow/tuple/wrap", [](auto check, auto &log) {
                auto c = planet::ui::breakable_row{
                        std::tuple{
                                planet::debug::fixed_element{log, {40, 3}},
                                planet::debug::fixed_element{log, {40, 3}},
                                planet::debug::fixed_element{log, {40, 3}}},
                        2, 5};
                c.reflow({.screen = screen}, screen);
                c.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{100, 100}});

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{82, 11}};
                check(std::get<0>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{40, 3}};
                check(std::get<1>(c.items).position())
                        == planet::affine::rectangle2d{
                                {57, 20}, planet::affine::extents2d{40, 3}};
                check(std::get<2>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 28}, planet::affine::extents2d{40, 3}};
            });


}
