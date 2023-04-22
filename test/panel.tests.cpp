#include <planet/ostream.hpp>
#include <planet/ui/panel.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("panel");


    auto const h1 = suite.test("hierarchy 1", [](auto check) {
        planet::ui::panel p, pc;
        planet::events::click c{};

        p.add_child(pc, {3, 4}, {5, 6});
        check(p.clicks.latest()).is_falsey();
        check(pc.clicks.latest()).is_falsey();

        c.location = {2, 3};
        p.clicks.push(c);
        check(p.clicks.latest()->location) == planet::affine::point2d{2, 3};
        check(pc.clicks.latest()).is_falsey();

        c.location = {4, 6};
        p.clicks.push(c);
        check(p.clicks.latest()->location) == planet::affine::point2d{4, 6};
        check(pc.clicks.latest()).is_truthy();
        auto const click = pc.clicks.latest()->location;
        check(click.x()) == 1;
        check(click.y()) == 2;
    });
    auto const h2 = suite.test("hierarchy 2", [](auto check) {
        planet::ui::panel p1, p3;
        planet::events::click c{};

        check(p1.clicks.latest()).is_falsey();
        check(p3.clicks.latest()).is_falsey();
        {
            planet::ui::panel p2;
            check(p2.clicks.latest()).is_falsey();
            p1.add_child(p2, {3, 4}, {15, 16});
            p2.add_child(p3, {1, 2}, {8, 9});

            c.location = {1, 2};
            p3.clicks.push(c);
            check(p3.clicks.latest()->location)
                    == planet::affine::point2d{1, 2};

            p2.clicks.push(c);
            check(p2.clicks.latest()->location)
                    == planet::affine::point2d{1, 2};
            check(p3.clicks.latest()->location)
                    == planet::affine::point2d{0, 0};

            c.location = {5, 8};
            p1.clicks.push(c);
            check(p1.clicks.latest()->location)
                    == planet::affine::point2d{5, 8};
            check(p2.clicks.latest()->location)
                    == planet::affine::point2d{2, 4};
            check(p3.clicks.latest()->location)
                    == planet::affine::point2d{1, 2};
        }
        p1.clicks.push(c);
        check(p1.clicks.latest()->location) == planet::affine::point2d{5, 8};
        /// p3 doesn't get the click
        check(p3.clicks.latest()->location) == planet::affine::point2d{1, 2};
    });


}
