#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/box.hpp>
#include <planet/ui/size.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;
    using axis_constrained_type = constrained_type::axis_constrained_type;


    auto const suite = felspar::testsuite("box.layout");


    auto const rf1 = suite.test("reflow/1", [](auto check) {
        constexpr planet::affine::extents2d target_size{40, 30};
        auto b = planet::ui::box{planet::ui::target_size{
                planet::debug::fixed_element{{4, 3}}, target_size}};
        b.reflow(
                {axis_constrained_type{0, 400, 400},
                 axis_constrained_type{0, 300, 300}});
        b.move_to({{0, 0}, planet::affine::extents2d{400, 300}});

        check(b.content.size) == target_size;
        check(b.content.constraints().min()) == target_size;
        check(b.position())
                == planet::affine::rectangle2d{
                        {0, 0}, planet::affine::extents2d{400, 300}};
        check(b.content.position())
                == planet::affine::rectangle2d{
                        {180, 135}, planet::affine::extents2d{40, 30}};
        check(b.content.content.position())
                == planet::affine::rectangle2d{
                        {180, 135}, planet::affine::extents2d{4, 3}};
    });
    auto const rf2 = suite.test("reflow/2", [](auto check) {
        constexpr planet::affine::extents2d target_size{40, 30};
        auto b = planet::ui::box{planet::ui::target_size{
                planet::ui::box{planet::debug::fixed_element{{4, 3}}},
                target_size}};
        b.reflow(
                {axis_constrained_type{0, 400, 400},
                 axis_constrained_type{0, 300, 300}});
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
