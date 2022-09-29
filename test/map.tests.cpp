#include <planet/map.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("map/coordinate", [](auto check) {
        planet::map::coordinate loc{};
        check(loc.row()) == 0;
        check(loc.column()) == 0;

        check(loc.e().column()) == 2;
        check(loc.e().row()) == 0;
        check(loc.ne().column()) == 1;
        check(loc.ne().row()) == 1;
        check(loc.nw().column()) == -1;
        check(loc.nw().row()) == 1;
        check(loc.w().column()) == -2;
        check(loc.w().row()) == 0;
        check(loc.sw().column()) == -1;
        check(loc.sw().row()) == -1;
        check(loc.se().column()) == 1;
        check(loc.se().row()) == -1;
    });


}
