#include <planet/map.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("map/coordinate", [](auto check) {
        planet::map::coordinate loc{};
        check(loc.row()) == 0;
        check(loc.column()) == 0;

        check((loc + planet::map::east).column()) == 2;
        check((loc + planet::map::east).row()) == 0;

        check((loc + planet::map::east + planet::map::north_east).column())
                == 3;
        check((loc + planet::map::east + planet::map::north_east).row()) == 1;

        check((loc + planet::map::east + planet::map::north_east
               + planet::map::north_west)
                      .column())
                == 2;
        check((loc + planet::map::east + planet::map::north_east
               + planet::map::north_west)
                      .row())
                == 2;

        check((loc + planet::map::east + planet::map::north_east
               + planet::map::north_west + planet::map::west)
                      .column())
                == 0;
        check((loc + planet::map::east + planet::map::north_east
               + planet::map::north_west + planet::map::west)
                      .row())
                == 2;

        check((loc + planet::map::east + planet::map::north_east
               + planet::map::north_west + planet::map::west
               + planet::map::south_west)
                      .column())
                == -1;
        check((loc + planet::map::east + planet::map::north_east
               + planet::map::north_west + planet::map::west
               + planet::map::south_west)
                      .row())
                == 1;

        check(loc + planet::map::east + planet::map::north_east
              + planet::map::north_west + planet::map::west
              + planet::map::south_west + planet::map::south_east)
                == loc;
    });


}
