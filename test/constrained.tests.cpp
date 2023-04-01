#include <planet/ui/constrained.hpp>
#include <felspar/test.hpp>


namespace {


    auto const c1d = felspar::testsuite("constrained1d");


    auto const n1d = c1d.test("comparisons", [](auto check) {
        check(planet::constrained1d{42}) == 42;
        check(42) == planet::constrained1d{42};
        check(planet::constrained1d{42}) == planet::constrained1d{42};

        check(planet::constrained1d{42}) != 43;
        check(42) != planet::constrained1d{43};
        check(planet::constrained1d{42}) != planet::constrained1d{43};

        check(planet::constrained1d{42}) != 43L;
        check(42) != planet::constrained1d{43L};
        check(planet::constrained1d{42}) != planet::constrained1d{43L};
    });


    auto const d1d = c1d.test("desired", [](auto check) {
        planet::constrained1d v1{42, {}, 100};
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


    auto const c2d = felspar::testsuite("constrained2d");


    auto const n2d = c2d.test("comparisons", [](auto check) {
        check(planet::constrained2d{3, 5}) == planet::constrained2d{3, 5};
    });


}
