#include <planet/debug/ui.hpp>
#include <planet/ui/baseplate.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("baseplate");


    auto const ordering = suite.test("ordering", [](auto check, auto &log) {
        planet::ui::panel panel;
        planet::ui::baseplate<std::ostream &> bp;
        planet::debug::button btn;

        check(bp.widget_count()) == 0u;
        check(btn.is_visible()) == false;

        btn.add_to(bp, panel);

        check(bp.widget_count()) == 1u;
        check(btn.is_visible()) == true;

        check([&]() {
            btn.draw(log);
        }).throws(std::logic_error{"Reflowable position has not been set"});

        check([&]() {
            btn.move_to({{15, 20}, planet::affine::extents2d{40, 30}});
        }).throws(std::logic_error{"Reflowable constraints have not been set"});
    });


}
