#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/layout.column.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;


    auto const suite = felspar::testsuite("column.layout");


    auto const arf1 = suite.test(
            "reflow/array",
            [](auto check) {
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
            },
            [](auto check) {
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


    auto const rf1 = suite.test(
            "reflow/tuple",
            [](auto check) {
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
            },
            [](auto check) {
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


    auto const vrf1 = suite.test("reflow/vector", [](auto check) {
        planet::ui::constrained2d<float> const constraints{
                {100, 0, 100}, {100, 0, 100}};
        planet::affine::rectangle2d const size{
                {13, 15}, planet::affine::extents2d{100, 100}};
        auto c =
                planet::ui::column{std::vector<planet::debug::fixed_element>{}};
        check(c.items.size()) == 0u;

        c.reflow(constraints);
        c.move_to(size);
        check(c.position().extents) == planet::affine::extents2d{};

        c.items.push_back({{4, 3}});
        c.reflow(constraints);
        c.move_to(size);
        check(c.position().top_left) == planet::affine::point2d{13, 15};
        check(c.position().extents) == planet::affine::extents2d{4, 3};
        check(c.items[0].position().top_left)
                == planet::affine::point2d{13, 15};
        check(c.items[0].position().extents) == planet::affine::extents2d{4, 3};

        c.items.push_back({{4, 3}});
        c.reflow(constraints);
        c.move_to(size);
        check(c.position().top_left) == planet::affine::point2d{13, 15};
        check(c.position().extents) == planet::affine::extents2d{4, 6};
        check(c.items[0].position().top_left)
                == planet::affine::point2d{13, 15};
        check(c.items[0].position().extents) == planet::affine::extents2d{4, 3};
        check(c.items[1].position().top_left)
                == planet::affine::point2d{13, 18};
        check(c.items[0].position().extents) == planet::affine::extents2d{4, 3};
    });


}
