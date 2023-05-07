#include <planet/ui/constrained.hpp>
#include <felspar/test.hpp>


namespace {


    auto const c1d = felspar::testsuite("constrained1d");


    auto const n1d = c1d.test("comparisons", [](auto check) {
        check(planet::ui::constrained1d{42}) == 42;
        check(42) == planet::ui::constrained1d{42};
        check(planet::ui::constrained1d{42}) == planet::ui::constrained1d{42};

        check(planet::ui::constrained1d{42}) != 43;
        check(42) != planet::ui::constrained1d{43};
        check(planet::ui::constrained1d{42}) != planet::ui::constrained1d{43};

        check(planet::ui::constrained1d{42}) != 43L;
        check(42) != planet::ui::constrained1d{43L};
        check(planet::ui::constrained1d{42}) != planet::ui::constrained1d{43L};

        check(planet::ui::constrained1d{42, 0, 100}
                      .is_at_least_as_constrained_as(
                              planet::ui::constrained1d{54, 0, 100}))
                == true;
        check(planet::ui::constrained1d{42, 10, 100}
                      .is_at_least_as_constrained_as(
                              planet::ui::constrained1d{54, 0, 100}))
                == true;
        check(planet::ui::constrained1d{42, 0, 90}.is_at_least_as_constrained_as(
                planet::ui::constrained1d{54, 0, 100}))
                == true;
        check(planet::ui::constrained1d{42, 0, 100}
                      .is_at_least_as_constrained_as(
                              planet::ui::constrained1d{54, 10, 100}))
                == false;
        check(planet::ui::constrained1d{42, 0, 100}
                      .is_at_least_as_constrained_as(
                              planet::ui::constrained1d{54, 0, 90}))
                == false;
    });


    auto const d1d = c1d.test("desired", [](auto check) {
        planet::ui::constrained1d v1{42, {}, 100};
        check(v1) == 42;

        v1.min(50);
        check(v1) == 50;
        v1.min(0);
        check(v1) == 42;

        v1.max(20);
        check(v1) == 20;
        v1.max(42);
        check(v1) == 42;
    });


    auto const l1d = c1d.test("normalisation", [](auto check) {
        planet::ui::constrained1d<float> db{-6, -128, 0};
        planet::ui::constrained1d<float> pixels{300, 200, 456};

        check(db.normalised_value()) == 122.0f / 128.0f;
        check(db.remapped_to(pixels)) == 444;

        check(pixels.normalised_value()) == 100.0f / 256.0f;
        check(pixels.remapped_to(db)) == -78.0f;
    });


    auto const c2d = felspar::testsuite("constrained2d");


    auto const n2d = c2d.test("comparisons", [](auto check) {
        check(planet::ui::constrained2d{3, 5})
                == planet::ui::constrained2d{3, 5};
    });


    auto const con2d = c2d.test("construction", [](auto check) {
        auto const c = planet::ui::constrained2d<float>{{4, 2, 10}, {6, 1, 8}};

        check(c.min()) == planet::affine::extents2d{2, 1};
        check(c.extents()) == planet::affine::extents2d{4, 6};
        check(c.max()) == planet::affine::extents2d{10, 8};
    });


}
