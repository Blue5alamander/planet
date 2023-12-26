#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/box.hpp>
#include <planet/ui/layout.grid.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;
    using axis_constrained_type = constrained_type::axis_constrained_type;


    auto const suite = felspar::testsuite("grid.layout");


    auto const rf1 = suite.test(
            "reflow",
            [](auto check, auto &log) {
                constexpr planet::affine::extents2d target_size{4, 3};
                auto g = planet::ui::grid{
                        std::tuple{
                                planet::debug::fixed_element{log, target_size}},
                        4};
                g.reflow({{400, 0, 400}, {300, 0, 300}});
                g.move_to({{0, 0}, planet::affine::extents2d{400, 300}});

                check(g.position())
                        == planet::affine::rectangle2d{
                                {0, 0}, planet::affine::extents2d{4, 3}};
                check(std::get<0>(g.items).position())
                        == planet::affine::rectangle2d{
                                {0, 0}, planet::affine::extents2d{4, 3}};
            },
            [](auto check, auto &log) {
                constexpr planet::affine::extents2d target_size{4, 3};
                auto g = planet::ui::grid{
                        std::tuple{
                                planet::debug::fixed_element{log, target_size},
                                planet::debug::fixed_element{log, target_size}},
                        4};
                g.reflow({{400, 0, 400}, {300, 0, 300}});
                g.move_to({{0, 0}, planet::affine::extents2d{400, 300}});

                check(g.hpadding) == 4;
                check(g.vpadding) == 4;

                check(g.position())
                        == planet::affine::rectangle2d{
                                {0, 0}, planet::affine::extents2d{12, 3}};
                check(std::get<0>(g.items).position())
                        == planet::affine::rectangle2d{
                                {0, 0}, planet::affine::extents2d{4, 3}};
                check(std::get<1>(g.items).position())
                        == planet::affine::rectangle2d{
                                {8, 0}, planet::affine::extents2d{4, 3}};
            },
            [](auto check, auto &log) {
                constexpr planet::affine::extents2d target_size{4, 3};
                auto g = planet::ui::grid{
                        std::tuple{
                                planet::debug::fixed_element{log, target_size},
                                planet::debug::fixed_element{log, target_size}},
                        4};
                g.reflow({{8, 0, 8}, {300, 0, 300}});
                g.move_to({{0, 0}, planet::affine::extents2d{8, 300}});

                check(g.hpadding) == 4;
                check(g.vpadding) == 4;

                check(g.position())
                        == planet::affine::rectangle2d{
                                {0, 0}, planet::affine::extents2d{4, 10}};
                check(std::get<0>(g.items).position())
                        == planet::affine::rectangle2d{
                                {0, 0}, planet::affine::extents2d{4, 3}};
                check(std::get<1>(g.items).position())
                        == planet::affine::rectangle2d{
                                {0, 7}, planet::affine::extents2d{4, 3}};
            },
            [](auto check, auto &log) {
                constexpr planet::affine::extents2d target_size{4, 3};
                auto g = planet::ui::box{planet::ui::grid{
                        std::tuple{
                                planet::debug::fixed_element{log, target_size},
                                planet::debug::fixed_element{log, target_size}},
                        5}};
                g.reflow({{400, 0, 400}, {300, 0, 300}});
                g.move_to({{0, 0}, planet::affine::extents2d{400, 300}});

                check(g.position())
                        == planet::affine::rectangle2d{
                                {0, 0}, planet::affine::extents2d{400, 300}};
                check(g.content.position())
                        == planet::affine::rectangle2d{
                                {193.5f, 148.5f},
                                planet::affine::extents2d{13, 3}};
                check(std::get<0>(g.content.items).position())
                        == planet::affine::rectangle2d{
                                {193.5f, 148.5f},
                                planet::affine::extents2d{4, 3}};
                check(std::get<1>(g.content.items).position())
                        == planet::affine::rectangle2d{
                                {202.5f, 148.5f},
                                planet::affine::extents2d{4, 3}};
            });


}
