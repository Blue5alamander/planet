#include <planet/ostream.hpp>
#include <planet/ui/panel.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("panel");


    auto const h1 = suite.test("hierarchy", [](auto check) {
        planet::ui::panel p, pc;
        p.add_child(pc, {3, 4}, {5, 6});

        check(p.into({2, 3})) == planet::affine::point2d{2, 3};
        check(p.into({4, 6})) == planet::affine::point2d{4, 6};
    });


}
