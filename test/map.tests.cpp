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


    auto const supercell = felspar::testsuite("map/supercell", [](auto check) {
        planet::map::supercell<
                planet::map::chunk<std::pair<std::size_t, std::size_t>, 2>, 2>
                c{[](auto x, auto y) {
                    return std::pair{x, y};
                }};

        check(c[{0, 0}]) == std::pair{0UL, 0UL};
        check(c[{1, 0}]) == std::pair{1UL, 0UL};
        check(c[{2, 0}]) == std::pair{2UL, 0UL};
        check(c[{3, 0}]) == std::pair{3UL, 0UL};

        check(c[{0, 1}]) == std::pair{0UL, 1UL};
        check(c[{1, 1}]) == std::pair{1UL, 1UL};
        check(c[{2, 1}]) == std::pair{2UL, 1UL};
        check(c[{3, 1}]) == std::pair{3UL, 1UL};

        check(c[{0, 2}]) == std::pair{0UL, 2UL};
        check(c[{1, 2}]) == std::pair{1UL, 2UL};
        check(c[{2, 2}]) == std::pair{2UL, 2UL};
        check(c[{3, 2}]) == std::pair{3UL, 2UL};

        check(c[{0, 3}]) == std::pair{0UL, 3UL};
        check(c[{1, 3}]) == std::pair{1UL, 3UL};
        check(c[{2, 3}]) == std::pair{2UL, 3UL};
        check(c[{3, 3}]) == std::pair{3UL, 3UL};
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
