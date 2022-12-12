#include <planet/ostream.hpp>
#include <felspar/test.hpp>


namespace {


    auto const coords = felspar::testsuite(
            "hexmap/coordinates",
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

                check(planet::hexmap::coordinates{4, 2}.column()) == 4;
                check(planet::hexmap::coordinates{4, 2}.row()) == 2;
                check(planet::hexmap::coordinates{5, 3}.column()) == 5;
                check(planet::hexmap::coordinates{5, 3}.row()) == 3;
                /// We move an illegal location to a legal one
                check(planet::hexmap::coordinates{5, 2}.column()) == 5;
                check(planet::hexmap::coordinates{5, 2}.row()) == 3;
            },
            [](auto check) {
                planet::hexmap::coordinates loc{};
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


    auto const col_iter = felspar::testsuite(
            "hexmap/coordinates/by_columns",
            [](auto check) {
                auto even =
                        planet::hexmap::coordinates::by_column({0, 0}, {0, 0});
                check(even.next()).is_falsey();

                auto odd =
                        planet::hexmap::coordinates::by_column({1, 1}, {1, 1});
                check(odd.next()).is_falsey();
            },
            [](auto check) {
                auto even =
                        planet::hexmap::coordinates::by_column({0, 2}, {2, 0});
                check(even.next().value()) == planet::hexmap::coordinates{0, 2};
                check(even.next().value()) == planet::hexmap::coordinates{1, 1};
                check(even.next()).is_falsey();

                auto odd =
                        planet::hexmap::coordinates::by_column({1, 3}, {3, 1});
                check(odd.next().value()) == planet::hexmap::coordinates{1, 3};
                check(odd.next().value()) == planet::hexmap::coordinates{2, 2};
                check(odd.next()).is_falsey();
            },
            [](auto check) {
                auto even = planet::hexmap::coordinates::by_column(
                        {-2, 2}, {3, -3});
                check(even.next().value())
                        == planet::hexmap::coordinates{-2, 2};
                check(even.next().value()) == planet::hexmap::coordinates{0, 2};
                check(even.next().value()) == planet::hexmap::coordinates{2, 2};
                check(even.next().value())
                        == planet::hexmap::coordinates{-1, 1};
                check(even.next().value()) == planet::hexmap::coordinates{1, 1};
                check(even.next().value())
                        == planet::hexmap::coordinates{-2, 0};
                check(even.next().value()) == planet::hexmap::coordinates{0, 0};
                check(even.next().value()) == planet::hexmap::coordinates{2, 0};
                check(even.next().value())
                        == planet::hexmap::coordinates{-1, -1};
                check(even.next().value())
                        == planet::hexmap::coordinates{1, -1};
                check(even.next().value())
                        == planet::hexmap::coordinates{-2, -2};
                check(even.next().value())
                        == planet::hexmap::coordinates{0, -2};
                check(even.next().value())
                        == planet::hexmap::coordinates{2, -2};
                check(even.next()).is_falsey();

                auto odd = planet::hexmap::coordinates::by_column(
                        {-1, 3}, {4, -2});
                check(odd.next().value()) == planet::hexmap::coordinates{-1, 3};
                check(odd.next().value()) == planet::hexmap::coordinates{1, 3};
                check(odd.next().value()) == planet::hexmap::coordinates{3, 3};
                check(odd.next().value()) == planet::hexmap::coordinates{0, 2};
                check(odd.next().value()) == planet::hexmap::coordinates{2, 2};
                check(odd.next().value()) == planet::hexmap::coordinates{-1, 1};
                check(odd.next().value()) == planet::hexmap::coordinates{1, 1};
                check(odd.next().value()) == planet::hexmap::coordinates{3, 1};
                check(odd.next().value()) == planet::hexmap::coordinates{0, 0};
                check(odd.next().value()) == planet::hexmap::coordinates{2, 0};
                check(odd.next().value())
                        == planet::hexmap::coordinates{-1, -1};
                check(odd.next().value()) == planet::hexmap::coordinates{1, -1};
                check(odd.next().value()) == planet::hexmap::coordinates{3, -1};
                check(odd.next()).is_falsey();
            });


    auto const world =
            felspar::testsuite("hexmap/world", [](auto check, auto &log) {
                std::size_t calls{};
                planet::hexmap::world_type<std::pair<long, long>, 4> w{
                        {0, 0}, [&calls](auto const p) mutable {
                            ++calls;
                            return std::pair{p.column(), p.row()};
                        }};
                check(calls) == 0;

                check(w[{0, 0}]) == std::pair{0L, 0L};
                check(calls) == 8;

                check(w[{5, 7}]) == std::pair{5L, 7L};
                check(calls) == 16;

                auto pos = w.chunks();
                auto p1 = pos.next()->first;
                log << "p1 " << p1.column() << ", " << p1.row() << '\n';
                check(p1) == planet::hexmap::coordinates{0, 0};
                auto p2 = pos.next()->first;
                log << "p2 " << p2.column() << ", " << p2.row() << '\n';
                check(p2) == planet::hexmap::coordinates{4, 4};
                check(pos.next()).is_falsey();
            });


    auto const distances = felspar::testsuite("hexmap/distances", [](auto check) {
        check(planet::hexmap::offset_of({0, 0})) == -1.0f;
        check(planet::hexmap::offset_of({0.5f, 0})) == -0.5f;
        check(std::abs(planet::hexmap::offset_of({1.0f, 0}))) <= 0.0e-5f;
        check(planet::hexmap::offset_of({1.5f, 0})) == 0.5f;
    });


}
