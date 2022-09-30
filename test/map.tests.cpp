#include <planet/map.hpp>
#include <felspar/test.hpp>


namespace {


    auto const chunk = felspar::testsuite("map/chunk", [](auto check) {
        planet::map::chunk<std::pair<std::size_t, std::size_t>, 2> c{
                [](auto x, auto y) {
                    return std::pair{x, y};
                }};

        check(c[{0, 0}]) == std::pair{0UL, 0UL};
        check(c[{1, 0}]) == std::pair{1UL, 0UL};
        check(c[{0, 1}]) == std::pair{0UL, 1UL};
        check(c[{1, 1}]) == std::pair{1UL, 1UL};
    });


    auto const coords = felspar::testsuite("map/coordinate", [](auto check) {
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
