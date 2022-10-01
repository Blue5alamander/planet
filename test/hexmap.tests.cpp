#include <planet/hexmap.hpp>
#include <felspar/test.hpp>


namespace {


    auto const coords = felspar::testsuite(
            "hexmap/coordinate",
            [](auto check) {
                check(planet::hexmap::east.column()) == 2;
                check(planet::hexmap::east.row()) == 0;
                check(planet::hexmap::north_east.column()) == 1;
                check(planet::hexmap::north_east.row()) == 1;
                check(planet::hexmap::north_west.column()) == -1;
                check(planet::hexmap::north_west.row()) == 1;
                check(planet::hexmap::west.column()) == -2;
                check(planet::hexmap::west.row()) == 0;
                check(planet::hexmap::south_west.column()) == -1;
                check(planet::hexmap::south_west.row()) == -1;
                check(planet::hexmap::south_east.column()) == 1;
                check(planet::hexmap::south_east.row()) == -1;

                check(planet::hexmap::coordinate{4, 2}.column()) == 4;
                check(planet::hexmap::coordinate{4, 2}.row()) == 2;
                check(planet::hexmap::coordinate{5, 3}.column()) == 5;
                check(planet::hexmap::coordinate{5, 3}.row()) == 3;
                /// We move an illegal location to a legal one
                check(planet::hexmap::coordinate{5, 2}.column()) == 5;
                check(planet::hexmap::coordinate{5, 2}.row()) == 3;
            },
            [](auto check) {
                planet::hexmap::coordinate loc{};
                check(loc.column()) == 0;
                check(loc.row()) == 0;

                check((loc + planet::hexmap::east).column()) == 2;
                check((loc + planet::hexmap::east).row()) == 0;

                check((loc + planet::hexmap::east + planet::hexmap::north_east)
                              .column())
                        == 3;
                check((loc + planet::hexmap::east + planet::hexmap::north_east)
                              .row())
                        == 1;

                check((loc + planet::hexmap::east + planet::hexmap::north_east
                       + planet::hexmap::north_west)
                              .column())
                        == 2;
                check((loc + planet::hexmap::east + planet::hexmap::north_east
                       + planet::hexmap::north_west)
                              .row())
                        == 2;

                check((loc + planet::hexmap::east + planet::hexmap::north_east
                       + planet::hexmap::north_west + planet::hexmap::west)
                              .column())
                        == 0;
                check((loc + planet::hexmap::east + planet::hexmap::north_east
                       + planet::hexmap::north_west + planet::hexmap::west)
                              .row())
                        == 2;

                check((loc + planet::hexmap::east + planet::hexmap::north_east
                       + planet::hexmap::north_west + planet::hexmap::west
                       + planet::hexmap::south_west)
                              .column())
                        == -1;
                check((loc + planet::hexmap::east + planet::hexmap::north_east
                       + planet::hexmap::north_west + planet::hexmap::west
                       + planet::hexmap::south_west)
                              .row())
                        == 1;

                check(loc + planet::hexmap::east + planet::hexmap::north_east
                      + planet::hexmap::north_west + planet::hexmap::west
                      + planet::hexmap::south_west + planet::hexmap::south_east)
                        == loc;
            });


}
