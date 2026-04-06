#include <planet/ui/target_aspect.hpp>
#include <felspar/test.hpp>


namespace {


    constexpr planet::ui::reflowable::constrained_type screen{
            {400, 0, 400}, {300, 0, 300}};

    auto const suite = felspar::testsuite("target_aspect");


    auto const height_limited = suite.test(
            "height-limited",
            [](auto check) {
                constexpr planet::affine::point2d position{};
                auto s = planet::ui::target_aspect{
                        planet::affine::extents2d{1, 1}};
                s.reflow({.screen = screen}, screen);
                s.move_to({.screen = screen}, {position, screen.extents()});
                check(s.position())
                        == planet::affine::rectangle2d{
                                position, planet::affine::extents2d{300, 300}};
            },
            [](auto check) {
                constexpr planet::affine::point2d position{};
                auto s = planet::ui::target_aspect{
                        planet::affine::extents2d{4, 3}};
                s.reflow({.screen = screen}, screen);
                s.move_to({.screen = screen}, {position, screen.extents()});
                check(s.position())
                        == planet::affine::rectangle2d{
                                position, planet::affine::extents2d{400, 300}};
            });


    auto const width_limited = suite.test("width-limited", [](auto check) {
        constexpr planet::affine::point2d position{};
        auto s = planet::ui::target_aspect{planet::affine::extents2d{2, 1}};
        s.reflow({.screen = screen}, screen);
        s.move_to({.screen = screen}, {position, screen.extents()});
        check(s.position())
                == planet::affine::rectangle2d{
                        position, planet::affine::extents2d{400, 200}};
    });


}
