#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/layout.breakable_row.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;


    auto const bsuite = felspar::testsuite("breakable_row.layout");


    auto const arf = bsuite.test(
            "reflow/array",
            [](auto check, auto &log) {
                auto c = planet::ui::breakable_row{
                        std::array{planet::debug::fixed_element{log, {4, 3}}},
                        2};
                c.reflow({{100, 0, 100}, {100, 0, 100}});
                c.move_to({{15, 20}, planet::affine::extents2d{100, 100}});

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
                c.reflow({{100, 0, 100}, {100, 0, 100}});
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


    auto const trf = bsuite.test(
            "reflow/tuple",
            [](auto check, auto &log) {
                auto c = planet::ui::breakable_row{
                        std::tuple{planet::debug::fixed_element{log, {4, 3}}},
                        2};
                c.reflow({{100, 0, 100}, {100, 0, 100}});
                c.move_to({{15, 20}, planet::affine::extents2d{100, 100}});

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
                c.reflow({{100, 0, 100}, {100, 0, 100}});
                c.move_to({{15, 20}, planet::affine::extents2d{100, 100}});

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


}
