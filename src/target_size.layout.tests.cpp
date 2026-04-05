#include <planet/ui/target_size.hpp>
#include <felspar/test.hpp>


namespace {


    constexpr planet::ui::reflowable::constrained_type screen{
            {400, 0, 400}, {300, 0, 300}};


    auto const suite = felspar::testsuite("target_size", [](auto check) {
        constexpr planet::affine::extents2d spacer_size{100, 50};
        constexpr planet::affine::extents2d bounds{400, 300};
        constexpr planet::affine::point2d position{50, 75};

        auto s = planet::ui::target_size{spacer_size};
        s.reflow({.screen = screen}, screen);
        s.move_to({.screen = screen}, {position, bounds});

        check(s.position())
                == planet::affine::rectangle2d{position, spacer_size};
        check(s.size) == spacer_size;
        check(s.constraints().min_extents()) == spacer_size;
        check(s.constraints().extents()) == spacer_size;
    });

}
