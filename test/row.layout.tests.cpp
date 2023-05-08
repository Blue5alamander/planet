#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/layout.row.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;


    auto const rsuite = felspar::testsuite("row.layout");


    auto const arf1 = rsuite.test(
            "reflow/array",
            [](auto check) {
                auto c = planet::ui::row{
                        std::array{planet::debug::fixed_element{{4, 3}}}, 2};
                c.reflow({{100, 0, 100}, {100, 0, 100}});
                c.move_to({{15, 20}, planet::affine::extents2d{100, 100}});

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
                check(c.items[0].position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
            },
            [](auto check) {
                auto c = planet::ui::row{
                        std::array{
                                planet::debug::fixed_element{{4, 3}},
                                planet::debug::fixed_element{{4, 3}}},
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


    auto const rf1 = rsuite.test("reflow/tuple", [](auto check) {
        auto c = planet::ui::row{
                std::tuple{planet::debug::fixed_element{{4, 3}}}, 2};
        c.reflow({{100, 0, 100}, {100, 0, 100}});
        c.move_to({{15, 20}, planet::affine::extents2d{100, 100}});

        check(c.position())
                == planet::affine::rectangle2d{
                        {15, 20}, planet::affine::extents2d{4, 3}};
        check(std::get<0>(c.items).position())
                == planet::affine::rectangle2d{
                        {15, 20}, planet::affine::extents2d{4, 3}};
    });


}
