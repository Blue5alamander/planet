#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/box.hpp>
#include <planet/ui/layout.grid.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;
    using axis_constrained_type = constrained_type::axis_constrained_type;


    auto const suite = felspar::testsuite("grid.layout");


    auto const rows1a = suite.test("one row/array", [](auto check) {
        auto r1 = planet::ui::grid{
                std::array{planet::debug::fixed_element{{3, 4}}}, {}};
        check(r1.extents({10, 10}).width) == 3.0f;
        check(r1.extents({10, 10}).height) == 4.0f;


        auto r2 = planet::ui::grid{
                std::array{
                        planet::debug::fixed_element{{3, 4}},
                        planet::debug::fixed_element{{3, 4}}},
                {}};
        check(r2.extents({10, 10}).width) == 6.0f;
        check(r2.extents({10, 10}).height) == 4.0f;

        r2.hpadding = 2.0f;
        r2.vpadding = 3.0f;
        check(r2.extents({10, 10}).width) == 8.0f;
        check(r2.extents({10, 10}).height) == 4.0f;


        auto r3 = planet::ui::grid{
                std::array{
                        planet::debug::fixed_element{{3, 4}},
                        planet::debug::fixed_element{{3, 4}},
                        planet::debug::fixed_element{{3, 4}}},
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


    auto const rows1t = suite.test("one row/tuple", [](auto check) {
        auto r1 = planet::ui::grid{
                std::tuple{planet::debug::fixed_element{{3, 4}}}, {}};
        check(r1.extents({10, 10}).width) == 3.0f;
        check(r1.extents({10, 10}).height) == 4.0f;


        auto r2 = planet::ui::grid{
                std::tuple{
                        planet::debug::fixed_element{{3, 4}},
                        planet::debug::fixed_element{{3, 4}}},
                {}};
        check(r2.extents({10, 10}).width) == 6.0f;
        check(r2.extents({10, 10}).height) == 4.0f;

        r2.hpadding = 2.0f;
        r2.vpadding = 3.0f;
        check(r2.extents({10, 10}).width) == 8.0f;
        check(r2.extents({10, 10}).height) == 4.0f;


        auto r3 = planet::ui::grid{
                std::tuple{
                        planet::debug::fixed_element{{3, 4}},
                        planet::debug::fixed_element{{3, 4}},
                        planet::debug::fixed_element{{3, 4}}},
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


    auto const rf1 = suite.test("reflow/1", [](auto check) {
        constexpr planet::affine::extents2d target_size{4, 3};
        auto g = planet::ui::grid{
                std::tuple{planet::debug::fixed_element{target_size}}, 4};
        g.reflow(
                {axis_constrained_type{0, 400, 400},
                 axis_constrained_type{0, 300, 300}});
        g.move_to({{0, 0}, planet::affine::extents2d{400, 300}});

        check(g.position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{400, 300}};
        check(std::get<0>(g.items).position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{4, 3}};
    });
    auto const rf2 = suite.test("reflow/2", [](auto check) {
        constexpr planet::affine::extents2d target_size{4, 3};
        auto g = planet::ui::grid{
                std::tuple{
                        planet::debug::fixed_element{target_size},
                        planet::debug::fixed_element{target_size}},
                4};
        g.reflow(
                {axis_constrained_type{0, 400, 400},
                 axis_constrained_type{0, 300, 300}});
        g.move_to({{0, 0}, planet::affine::extents2d{400, 300}});

        check(g.hpadding) == 4;
        check(g.vpadding) == 4;

        check(g.position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{400, 300}};
        check(std::get<0>(g.items).position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{4, 3}};
        check(std::get<1>(g.items).position())
                == planet::affine::rectangle2d{
                        {8, 0}, planet::affine::extents2d{4, 3}};
    });
    auto const rf3 = suite.test("reflow/3", [](auto check) {
        constexpr planet::affine::extents2d target_size{4, 3};
        auto g = planet::ui::grid{
                std::tuple{
                        planet::debug::fixed_element{target_size},
                        planet::debug::fixed_element{target_size}},
                4};
        g.reflow(
                {axis_constrained_type{0, 8, 8},
                 axis_constrained_type{0, 300, 300}});
        g.move_to({{0, 0}, planet::affine::extents2d{8, 300}});

        check(g.hpadding) == 4;
        check(g.vpadding) == 4;

        check(g.position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{8, 300}};
        check(std::get<0>(g.items).position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{4, 3}};
        check(std::get<1>(g.items).position())
                == planet::affine::rectangle2d{
                        {0, 7}, planet::affine::extents2d{4, 3}};
    });
    auto const rf4 = suite.test("reflow/4", [](auto check) {
        constexpr planet::affine::extents2d target_size{4, 3};
        auto g = planet::ui::box{planet::ui::grid{
                std::tuple{
                        planet::debug::fixed_element{target_size},
                        planet::debug::fixed_element{target_size}},
                5}};
        g.reflow(
                {axis_constrained_type{0, 400, 400},
                 axis_constrained_type{0, 300, 300}});
        g.move_to({{0, 0}, planet::affine::extents2d{400, 300}});

        check(g.position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{400, 300}};
        check(g.content.position())
                == planet::affine::rectangle2d{
                        {193.5f, 148.5f}, planet::affine::extents2d{13, 3}};
        check(std::get<0>(g.content.items).position())
                == planet::affine::rectangle2d{
                        {193.5f, 148.5f}, planet::affine::extents2d{4, 3}};
        check(std::get<1>(g.content.items).position())
                == planet::affine::rectangle2d{
                        {202.5f, 148.5f}, planet::affine::extents2d{4, 3}};
    });


}
