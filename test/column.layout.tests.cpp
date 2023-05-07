#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/layout.column.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;


    auto const suite = felspar::testsuite("column.layout");


    auto const arf1 = suite.test("reflow/array/1", [](auto check) {
        auto c = planet::ui::column{
                std::array{planet::debug::fixed_element{{4, 3}}}, 2};
        c.reflow({{100, 0, 100}, {100, 0, 100}});
        c.move_to({{0, 0}, planet::affine::extents2d{100, 100}});

        check(c.position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{4, 3}};
        check(std::get<0>(c.items).position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{4, 3}};
    });
    auto const arf2 = suite.test("reflow/array/2", [](auto check) {
        auto c = planet::ui::column{
                std::array{
                        planet::debug::fixed_element{{4, 3}},
                        planet::debug::fixed_element{{4, 3}}},
                2};
        c.reflow({{100, 0, 100}, {100, 0, 100}});
        c.move_to({{13, 15}, planet::affine::extents2d{100, 100}});

        check(c.position())
                == planet::affine::rectangle2d{
                        {13, 15}, planet::affine::extents2d{4, 8}};
        check(std::get<0>(c.items).position())
                == planet::affine::rectangle2d{
                        {13, 15}, planet::affine::extents2d{4, 3}};
        check(std::get<1>(c.items).position())
                == planet::affine::rectangle2d{
                        {13, 20}, planet::affine::extents2d{4, 3}};
    });


    auto const rf1 = suite.test("reflow/tuple/1", [](auto check) {
        auto c = planet::ui::column{
                std::tuple{planet::debug::fixed_element{{4, 3}}}, 2};
        c.reflow({{100, 0, 100}, {100, 0, 100}});
        c.move_to({{0, 0}, planet::affine::extents2d{100, 100}});

        check(c.position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{4, 3}};
        check(std::get<0>(c.items).position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{4, 3}};
    });
    auto const rf2 = suite.test("reflow/tuple/2", [](auto check) {
        auto c = planet::ui::column{
                std::tuple{
                        planet::debug::fixed_element{{4, 3}},
                        planet::debug::fixed_element{{4, 3}}},
                2};
        c.reflow({{100, 0, 100}, {100, 0, 100}});
        c.move_to({{13, 15}, planet::affine::extents2d{100, 100}});

        check(c.position())
                == planet::affine::rectangle2d{
                        {13, 15}, planet::affine::extents2d{4, 8}};
        check(std::get<0>(c.items).position())
                == planet::affine::rectangle2d{
                        {13, 15}, planet::affine::extents2d{4, 3}};
        check(std::get<1>(c.items).position())
                == planet::affine::rectangle2d{
                        {13, 20}, planet::affine::extents2d{4, 3}};
    });


}
