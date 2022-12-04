#include <planet/ui/scale.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("scaling");


    constexpr planet::affine::extents2d unit{1, 1}, landscape{4, 2},
            portrait{2, 4};


    auto const n = suite.test("never", [](auto check) {
        check(planet::ui::scaling(unit, landscape, planet::ui::scale::never)
                      .width)
                == 1.0f;
        check(planet::ui::scaling(unit, landscape, planet::ui::scale::never)
                      .height)
                == 1.0f;
        check(planet::ui::scaling(unit, portrait, planet::ui::scale::never)
                      .width)
                == 1.0f;
        check(planet::ui::scaling(unit, portrait, planet::ui::scale::never)
                      .height)
                == 1.0f;


        check(planet::ui::scaling(landscape, unit, planet::ui::scale::never)
                      .width)
                == 4.0f;
        check(planet::ui::scaling(landscape, unit, planet::ui::scale::never)
                      .height)
                == 2.0f;
        check(planet::ui::scaling(landscape, portrait, planet::ui::scale::never)
                      .width)
                == 4.0f;
        check(planet::ui::scaling(landscape, portrait, planet::ui::scale::never)
                      .height)
                == 2.0f;


        check(planet::ui::scaling(portrait, unit, planet::ui::scale::never)
                      .width)
                == 2.0f;
        check(planet::ui::scaling(portrait, unit, planet::ui::scale::never)
                      .height)
                == 4.0f;
        check(planet::ui::scaling(portrait, landscape, planet::ui::scale::never)
                      .width)
                == 2.0f;
        check(planet::ui::scaling(portrait, landscape, planet::ui::scale::never)
                      .height)
                == 4.0f;
    });


    auto const ex = suite.test("expand/x", [](auto check) {
        check(planet::ui::scaling(unit, landscape, planet::ui::scale::expand_x)
                      .width)
                == 4.0f;
        check(planet::ui::scaling(unit, landscape, planet::ui::scale::expand_x)
                      .height)
                == 1.0f;
        check(planet::ui::scaling(unit, portrait, planet::ui::scale::expand_x)
                      .width)
                == 2.0f;
        check(planet::ui::scaling(unit, portrait, planet::ui::scale::expand_x)
                      .height)
                == 1.0f;


        check(planet::ui::scaling(landscape, unit, planet::ui::scale::expand_x)
                      .width)
                == 4.0f;
        check(planet::ui::scaling(landscape, unit, planet::ui::scale::expand_x)
                      .height)
                == 2.0f;
        check(planet::ui::scaling(
                      landscape, portrait, planet::ui::scale::expand_x)
                      .width)
                == 4.0f;
        check(planet::ui::scaling(
                      landscape, portrait, planet::ui::scale::expand_x)
                      .height)
                == 2.0f;


        check(planet::ui::scaling(portrait, unit, planet::ui::scale::expand_x)
                      .width)
                == 2.0f;
        check(planet::ui::scaling(portrait, unit, planet::ui::scale::expand_x)
                      .height)
                == 4.0f;
        check(planet::ui::scaling(
                      portrait, landscape, planet::ui::scale::expand_x)
                      .width)
                == 4.0f;
        check(planet::ui::scaling(
                      portrait, landscape, planet::ui::scale::expand_x)
                      .height)
                == 4.0f;
    });


    auto const ey = suite.test("expand/y", [](auto check) {
        check(planet::ui::scaling(unit, landscape, planet::ui::scale::expand_y)
                      .width)
                == 1.0f;
        check(planet::ui::scaling(unit, landscape, planet::ui::scale::expand_y)
                      .height)
                == 2.0f;
        check(planet::ui::scaling(unit, portrait, planet::ui::scale::expand_y)
                      .width)
                == 1.0f;
        check(planet::ui::scaling(unit, portrait, planet::ui::scale::expand_y)
                      .height)
                == 4.0f;


        check(planet::ui::scaling(landscape, unit, planet::ui::scale::expand_y)
                      .width)
                == 4.0f;
        check(planet::ui::scaling(landscape, unit, planet::ui::scale::expand_y)
                      .height)
                == 2.0f;
        check(planet::ui::scaling(
                      landscape, portrait, planet::ui::scale::expand_y)
                      .width)
                == 4.0f;
        check(planet::ui::scaling(
                      landscape, portrait, planet::ui::scale::expand_y)
                      .height)
                == 4.0f;


        check(planet::ui::scaling(portrait, unit, planet::ui::scale::expand_y)
                      .width)
                == 2.0f;
        check(planet::ui::scaling(portrait, unit, planet::ui::scale::expand_y)
                      .height)
                == 4.0f;
        check(planet::ui::scaling(
                      portrait, landscape, planet::ui::scale::expand_y)
                      .width)
                == 2.0f;
        check(planet::ui::scaling(
                      portrait, landscape, planet::ui::scale::expand_y)
                      .height)
                == 4.0f;
    });


    auto const sx = suite.test("shrink/x", [](auto check) {
        check(planet::ui::scaling(unit, landscape, planet::ui::scale::shrink_x)
                      .width)
                == 1.0f;
        check(planet::ui::scaling(unit, landscape, planet::ui::scale::shrink_x)
                      .height)
                == 1.0f;
        check(planet::ui::scaling(unit, portrait, planet::ui::scale::shrink_x)
                      .width)
                == 1.0f;
        check(planet::ui::scaling(unit, portrait, planet::ui::scale::shrink_x)
                      .height)
                == 1.0f;


        check(planet::ui::scaling(landscape, unit, planet::ui::scale::shrink_x)
                      .width)
                == 1.0f;
        check(planet::ui::scaling(landscape, unit, planet::ui::scale::shrink_x)
                      .height)
                == 2.0f;
        check(planet::ui::scaling(
                      landscape, portrait, planet::ui::scale::shrink_x)
                      .width)
                == 2.0f;
        check(planet::ui::scaling(
                      landscape, portrait, planet::ui::scale::shrink_x)
                      .height)
                == 2.0f;


        check(planet::ui::scaling(portrait, unit, planet::ui::scale::shrink_x)
                      .width)
                == 1.0f;
        check(planet::ui::scaling(portrait, unit, planet::ui::scale::shrink_x)
                      .height)
                == 4.0f;
        check(planet::ui::scaling(
                      portrait, landscape, planet::ui::scale::shrink_x)
                      .width)
                == 2.0f;
        check(planet::ui::scaling(
                      portrait, landscape, planet::ui::scale::shrink_x)
                      .height)
                == 4.0f;
    });


    auto const sy = suite.test("shrink/y", [](auto check) {
        check(planet::ui::scaling(unit, landscape, planet::ui::scale::shrink_y)
                      .width)
                == 1.0f;
        check(planet::ui::scaling(unit, landscape, planet::ui::scale::shrink_y)
                      .height)
                == 1.0f;
        check(planet::ui::scaling(unit, portrait, planet::ui::scale::shrink_y)
                      .width)
                == 1.0f;
        check(planet::ui::scaling(unit, portrait, planet::ui::scale::shrink_y)
                      .height)
                == 1.0f;


        check(planet::ui::scaling(landscape, unit, planet::ui::scale::shrink_y)
                      .width)
                == 4.0f;
        check(planet::ui::scaling(landscape, unit, planet::ui::scale::shrink_y)
                      .height)
                == 1.0f;
        check(planet::ui::scaling(
                      landscape, portrait, planet::ui::scale::shrink_y)
                      .width)
                == 4.0f;
        check(planet::ui::scaling(
                      landscape, portrait, planet::ui::scale::shrink_y)
                      .height)
                == 2.0f;


        check(planet::ui::scaling(portrait, unit, planet::ui::scale::shrink_y)
                      .width)
                == 2.0f;
        check(planet::ui::scaling(portrait, unit, planet::ui::scale::shrink_y)
                      .height)
                == 1.0f;
        check(planet::ui::scaling(
                      portrait, landscape, planet::ui::scale::shrink_y)
                      .width)
                == 2.0f;
        check(planet::ui::scaling(
                      portrait, landscape, planet::ui::scale::shrink_y)
                      .height)
                == 2.0f;
    });


}
