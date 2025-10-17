#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/box.hpp>
#include <planet/ui/target_size.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;

    constexpr planet::ui::reflowable::constrained_type screen{
            {400, 0, 400}, {300, 0, 300}};


    auto const suite = felspar::testsuite("box.layout");


    auto const rf = suite.test(
            "reflow",
            [](auto check, auto &log) {
                constexpr planet::affine::extents2d size{4, 3};
                constexpr planet::affine::extents2d bounds{400, 300};

                auto b = planet::ui::box{
                        planet::debug::fixed_element{log, size}, 2, 2};
                b.reflow({.screen = screen}, screen);
                b.move_to({.screen = screen}, {{15, 20}, bounds});

                check(b.content.constraints().min_extents()) == size;
                check(b.content.constraints().extents()) == size;
                check(b.content.constraints().max_extents()) == size;

                check(b.constraints().min_extents())
                        == planet::affine::extents2d{8, 7};
                check(b.constraints().extents())
                        == planet::affine::extents2d{8, 7};
                check(b.constraints().max_extents()) == bounds;
            },
            [](auto check, auto &log) {
                constexpr planet::affine::extents2d target_size{40, 30};
                constexpr planet::affine::extents2d bounds{400, 300};

                auto b = planet::ui::box{planet::ui::target_size{
                        planet::debug::fixed_element{log, {4, 3}}, target_size}};
                b.reflow({.screen = screen}, screen);

                check(b.content.size) == target_size;
                check(b.content.constraints().min_extents()) == target_size;
                check(b.constraints().min_extents()) == target_size;

                b.move_to({.screen = screen}, {{0, 0}, bounds});

                check(b.position())
                        == planet::affine::rectangle2d{{0, 0}, bounds};
                check(b.content.position())
                        == planet::affine::rectangle2d{
                                {180, 135}, planet::affine::extents2d{40, 30}};
                check(b.content.content.position())
                        == planet::affine::rectangle2d{
                                {180, 135}, planet::affine::extents2d{4, 3}};
            },
            [](auto check, auto &log) {
                constexpr planet::affine::extents2d target_size{40, 30};
                auto b = planet::ui::box{planet::ui::target_size{
                        planet::ui::box{
                                planet::debug::fixed_element{log, {4, 3}}},
                        target_size}};
                b.reflow({.screen = screen}, screen);
                b.move_to(
                        {.screen = screen},
                        {{0, 0}, planet::affine::extents2d{400, 300}});

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
