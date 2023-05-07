#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/box.hpp>
#include <planet/ui/size.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;


    auto const suite = felspar::testsuite("box.layout");


    auto const rf1 = suite.test("reflow/1", [](auto check) {
        constexpr planet::affine::extents2d size{4, 3};
        constexpr planet::affine::extents2d bounds{400, 300};

        auto b = planet::ui::box{planet::debug::fixed_element{size}, 2, 2};
        b.reflow({{400, 0, 400}, {300, 0, 300}});
        b.move_to({{15, 20}, bounds});

        check(b.content.constraints().min()) == size;
        check(b.content.constraints().extents()) == size;
        check(b.content.constraints().max()) == size;

        check(b.constraints().min()) == planet::affine::extents2d{8, 7};
        check(b.constraints().extents()) == bounds;
        check(b.constraints().max()) == bounds;
    });
    auto const rf2 = suite.test("reflow/2", [](auto check) {
        constexpr planet::affine::extents2d target_size{40, 30};
        constexpr planet::affine::extents2d bounds{400, 300};

        auto b = planet::ui::box{planet::ui::target_size{
                planet::debug::fixed_element{{4, 3}}, target_size}};
        b.reflow({{400, 0, 400}, {300, 0, 300}});

        check(b.content.size) == target_size;
        check(b.content.constraints().min()) == target_size;
        check(b.constraints().min()) == target_size;

        b.move_to({{0, 0}, bounds});

        check(b.position()) == planet::affine::rectangle2d{{0, 0}, bounds};
        check(b.content.position())
                == planet::affine::rectangle2d{
                        {180, 135}, planet::affine::extents2d{40, 30}};
        check(b.content.content.position())
                == planet::affine::rectangle2d{
                        {180, 135}, planet::affine::extents2d{4, 3}};
    });
    auto const rf3 = suite.test("reflow/3", [](auto check) {
        constexpr planet::affine::extents2d target_size{40, 30};
        auto b = planet::ui::box{planet::ui::target_size{
                planet::ui::box{planet::debug::fixed_element{{4, 3}}},
                target_size}};
        b.reflow({{400, 0, 400}, {300, 0, 300}});
        b.move_to({{0, 0}, planet::affine::extents2d{400, 300}});

        check(b.position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{400, 300}};
        check(b.content.position())
                == planet::affine::rectangle2d{
                        {180, 135}, planet::affine::extents2d{40, 30}};
        check(b.content.content.position())
                == planet::affine::rectangle2d{
                        {180, 135}, planet::affine::extents2d{40, 30}};
        check(b.content.content.content.position())
                == planet::affine::rectangle2d{
                        {198, 148.5f}, planet::affine::extents2d{4, 3}};
    });


}
