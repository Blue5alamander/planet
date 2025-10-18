#include <planet/ostream.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("map::hex");


    auto const coords = suite.test(
            "coordinates",
            [](auto check) {
                check(planet::map::hex::east.column()) == 2;
                check(planet::map::hex::east.row()) == 0;
                check(planet::map::hex::north_east.column()) == 1;
                check(planet::map::hex::north_east.row()) == 1;
                check(planet::map::hex::north_west.column()) == -1;
                check(planet::map::hex::north_west.row()) == 1;
                check(planet::map::hex::west.column()) == -2;
                check(planet::map::hex::west.row()) == 0;
                check(planet::map::hex::south_west.column()) == -1;
                check(planet::map::hex::south_west.row()) == -1;
                check(planet::map::hex::south_east.column()) == 1;
                check(planet::map::hex::south_east.row()) == -1;

                check(planet::map::hex::coordinates{4, 2}.column()) == 4;
                check(planet::map::hex::coordinates{4, 2}.row()) == 2;
                check(planet::map::hex::coordinates{5, 3}.column()) == 5;
                check(planet::map::hex::coordinates{5, 3}.row()) == 3;
                /// We move an illegal location to a legal one
                check(planet::map::hex::coordinates{5, 2}.column()) == 5;
                check(planet::map::hex::coordinates{5, 2}.row()) == 3;
            },
            [](auto check) {
                planet::map::hex::coordinates loc{};
                check(loc.column()) == 0;
                check(loc.row()) == 0;

                check((loc + planet::map::hex::east).column()) == 2;
                check((loc + planet::map::hex::east).row()) == 0;

                check((loc + planet::map::hex::east
                       + planet::map::hex::north_east)
                              .column())
                        == 3;
                check((loc + planet::map::hex::east
                       + planet::map::hex::north_east)
                              .row())
                        == 1;

                check((loc + planet::map::hex::east
                       + planet::map::hex::north_east
                       + planet::map::hex::north_west)
                              .column())
                        == 2;
                check((loc + planet::map::hex::east
                       + planet::map::hex::north_east
                       + planet::map::hex::north_west)
                              .row())
                        == 2;

                check((loc + planet::map::hex::east
                       + planet::map::hex::north_east
                       + planet::map::hex::north_west + planet::map::hex::west)
                              .column())
                        == 0;
                check((loc + planet::map::hex::east
                       + planet::map::hex::north_east
                       + planet::map::hex::north_west + planet::map::hex::west)
                              .row())
                        == 2;

                check((loc + planet::map::hex::east
                       + planet::map::hex::north_east
                       + planet::map::hex::north_west + planet::map::hex::west
                       + planet::map::hex::south_west)
                              .column())
                        == -1;
                check((loc + planet::map::hex::east
                       + planet::map::hex::north_east
                       + planet::map::hex::north_west + planet::map::hex::west
                       + planet::map::hex::south_west)
                              .row())
                        == 1;

                check(loc + planet::map::hex::east
                      + planet::map::hex::north_east
                      + planet::map::hex::north_west + planet::map::hex::west
                      + planet::map::hex::south_west
                      + planet::map::hex::south_east)
                        == loc;
            });


    auto const col_iter = suite.test(
            "coordinates/by_columns",
            [](auto check) {
                auto even = planet::map::hex::coordinates::by_column(
                        {0, 0}, {0, 0});
                check(even.next()).is_falsey();

                auto odd = planet::map::hex::coordinates::by_column(
                        {1, 1}, {1, 1});
                check(odd.next()).is_falsey();
            },
            [](auto check) {
                auto even = planet::map::hex::coordinates::by_column(
                        {0, 2}, {2, 0});
                check(even.next().value())
                        == planet::map::hex::coordinates{0, 2};
                check(even.next().value())
                        == planet::map::hex::coordinates{1, 1};
                check(even.next()).is_falsey();

                auto odd = planet::map::hex::coordinates::by_column(
                        {1, 3}, {3, 1});
                check(odd.next().value())
                        == planet::map::hex::coordinates{1, 3};
                check(odd.next().value())
                        == planet::map::hex::coordinates{2, 2};
                check(odd.next()).is_falsey();
            },
            [](auto check) {
                auto even = planet::map::hex::coordinates::by_column(
                        {-2, 2}, {3, -3});
                check(even.next().value())
                        == planet::map::hex::coordinates{-2, 2};
                check(even.next().value())
                        == planet::map::hex::coordinates{0, 2};
                check(even.next().value())
                        == planet::map::hex::coordinates{2, 2};
                check(even.next().value())
                        == planet::map::hex::coordinates{-1, 1};
                check(even.next().value())
                        == planet::map::hex::coordinates{1, 1};
                check(even.next().value())
                        == planet::map::hex::coordinates{-2, 0};
                check(even.next().value())
                        == planet::map::hex::coordinates{0, 0};
                check(even.next().value())
                        == planet::map::hex::coordinates{2, 0};
                check(even.next().value())
                        == planet::map::hex::coordinates{-1, -1};
                check(even.next().value())
                        == planet::map::hex::coordinates{1, -1};
                check(even.next().value())
                        == planet::map::hex::coordinates{-2, -2};
                check(even.next().value())
                        == planet::map::hex::coordinates{0, -2};
                check(even.next().value())
                        == planet::map::hex::coordinates{2, -2};
                check(even.next()).is_falsey();

                auto odd = planet::map::hex::coordinates::by_column(
                        {-1, 3}, {4, -2});
                check(odd.next().value())
                        == planet::map::hex::coordinates{-1, 3};
                check(odd.next().value())
                        == planet::map::hex::coordinates{1, 3};
                check(odd.next().value())
                        == planet::map::hex::coordinates{3, 3};
                check(odd.next().value())
                        == planet::map::hex::coordinates{0, 2};
                check(odd.next().value())
                        == planet::map::hex::coordinates{2, 2};
                check(odd.next().value())
                        == planet::map::hex::coordinates{-1, 1};
                check(odd.next().value())
                        == planet::map::hex::coordinates{1, 1};
                check(odd.next().value())
                        == planet::map::hex::coordinates{3, 1};
                check(odd.next().value())
                        == planet::map::hex::coordinates{0, 0};
                check(odd.next().value())
                        == planet::map::hex::coordinates{2, 0};
                check(odd.next().value())
                        == planet::map::hex::coordinates{-1, -1};
                check(odd.next().value())
                        == planet::map::hex::coordinates{1, -1};
                check(odd.next().value())
                        == planet::map::hex::coordinates{3, -1};
                check(odd.next()).is_falsey();
            });


    auto const dirs = suite.test("directions", [](auto check) {
        planet::map::hex::coordinates const from{5, 7};
        check(best_direction(from, {11, 7})) == planet::map::hex::east;
        check(best_direction(from, {11, 13})) == planet::map::hex::north_east;
        check(best_direction(from, {-1, 13})) == planet::map::hex::north_west;
        check(best_direction(from, {-1, 7})) == planet::map::hex::west;
        check(best_direction(from, {-1, 1})) == planet::map::hex::south_west;
        check(best_direction(from, {11, 1})) == planet::map::hex::south_east;

        check(planet::map::hex::best_direction({2, 0}, {}))
                == planet::map::hex::coordinates{-2, 0};
    });


    auto const world = suite.test("world", [](auto check, auto &log) {
        std::size_t calls{};
        planet::map::hex::world_type<std::pair<long, long>, 4> w{
                {0, 0}, [&calls](auto const p) mutable {
                    ++calls;
                    return std::pair{p.column(), p.row()};
                }};
        check(calls) == 0u;

        check(w[{0, 0}]) == std::pair{0L, 0L};
        check(calls) == 8u;

        check(w[{5, 7}]) == std::pair{5L, 7L};
        check(calls) == 16u;

        auto pos = w.chunks();
        auto p1 = pos.next()->first;
        log << "p1 " << p1.column() << ", " << p1.row() << '\n';
        check(p1) == planet::map::hex::coordinates{0, 0};
        auto p2 = pos.next()->first;
        log << "p2 " << p2.column() << ", " << p2.row() << '\n';
        check(p2) == planet::map::hex::coordinates{4, 4};
        check(pos.next()).is_falsey();
    });


    auto const moves = suite.test("moves", [](auto check) {
        check(planet::map::hex::coordinates{0, 0}.move_distance()) == 0u;
        check(planet::map::hex::coordinates{1, 1}.move_distance()) == 1u;
        check(planet::map::hex::coordinates{2, 0}.move_distance()) == 1u;
        check(planet::map::hex::coordinates{3, 1}.move_distance()) == 2u;
        check(planet::map::hex::coordinates{5, 1}.move_distance()) == 3u;
        check(planet::map::hex::coordinates{6, 2}.move_distance()) == 4u;
    });


    auto const distances = suite.test("signed_distance", [](auto check) {
        check(planet::map::hex::signed_distance({0, 0})) == -1.0f;
        check(planet::map::hex::signed_distance({0.5f, 0})) == -0.5f;
        check(std::abs(planet::map::hex::signed_distance({1.0f, 0})))
                <= 0.0e-5f;
        check(planet::map::hex::signed_distance({1.5f, 0})) == 0.5f;
    });


    auto const turning = suite.test(
            "turning",
            [](auto check) {
                check(planet::map::hex::direction_to_rotation(
                        planet::map::hex::east_index))
                        == 0.0f;
                check(planet::map::hex::direction_to_rotation(
                        planet::map::hex::west_index))
                        == 0.5f;
            },
            [](auto check) {
                check(planet::map::hex::tween_rotation(0, 1).from) == 0.0f;
                check(planet::map::hex::tween_rotation(0, 1).to) == 1.0f / 6.0f;

                check(planet::map::hex::tween_rotation(1, 0).from)
                        == 1.0f / 6.0f;
                check(planet::map::hex::tween_rotation(1, 0).to) == 0.0f;

                check(planet::map::hex::tween_rotation(0, 5).from) == 0.0f;
                check(planet::map::hex::tween_rotation(0, 5).to)
                        == -1.0f / 6.0f;

                check(planet::map::hex::tween_rotation(5, 0).from)
                        == 5.0f / 6.0f;
                check(planet::map::hex::tween_rotation(5, 0).to) == 1.0f;
            });


}
