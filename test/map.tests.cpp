#include <planet/map.hpp>
#include <felspar/test.hpp>

#include <felspar/coro/eager.hpp>


namespace {


    auto const chunk = felspar::testsuite("map/chunk", [](auto check) {
        planet::map::chunk<std::pair<std::size_t, std::size_t>, 2> c{
                [](auto x, auto y) { return std::pair{x, y}; }};

        check(c[{0, 0}]) == std::pair{std::size_t{}, std::size_t{}};
        check(c[{1, 0}]) == std::pair{std::size_t{1}, std::size_t{}};
        check(c[{0, 1}]) == std::pair{std::size_t{}, std::size_t{1}};
        check(c[{1, 1}]) == std::pair{std::size_t{1}, std::size_t{1}};
    });


    auto const coords = felspar::testsuite(
            "map/coordinates",
            [](auto check) {
                planet::map::coordinates loc{};
                check(loc.row()) == 0;
                check(loc.column()) == 0;
            },
            [](auto check) {
                check(planet::map::coordinates::insert_count(0, 4, 8)) == 0u;
                check(planet::map::coordinates::insert_count(4, 0, 8)) == 1u;
                check(planet::map::coordinates::insert_count(0, -4, 8)) == 1u;
                check(planet::map::coordinates::insert_count(0, -7, 8)) == 1u;
                check(planet::map::coordinates::insert_count(0, -8, 8)) == 2u;
            },
            [](auto check) {
                check(planet::map::coordinates::chunk_number(0, 0, 8)) == 0UL;
                check(planet::map::coordinates::chunk_number(-8, -4, 8)) == 0UL;
                check(planet::map::coordinates::chunk_number(-8, 0, 8)) == 1UL;
                check(planet::map::coordinates::chunk_number(-8, -1, 8)) == 0UL;
            },
            [](auto check) {
                check(planet::map::coordinates::inside_chunk(0, 4, 8)) == 4UL;
                check(planet::map::coordinates::inside_chunk(0, 7, 8)) == 7UL;
                check(planet::map::coordinates::inside_chunk(0, 8, 8)) == 0UL;
                check(planet::map::coordinates::inside_chunk(0, 14, 8)) == 6UL;
                check(planet::map::coordinates::inside_chunk(-8, -1, 8)) == 7UL;
                check(planet::map::coordinates::inside_chunk(-16, -1, 8))
                        == 7UL;
            },
            [](auto check) {
                check(planet::map::coordinates{3, 4}.mag2()) == 25;
            });


    auto const world = felspar::testsuite(
            "map/world",
            [](auto check) {
                std::size_t calls{};
                planet::map::world<planet::map::chunk<std::pair<long, long>, 4>>
                        w{{0, 0}, [&calls](auto const p) mutable {
                              ++calls;
                              return std::pair{p.column(), p.row()};
                          }};
                check(calls) == 0u;

                check(w[{0, 0}]) == std::pair{0L, 0L};
                check(calls) == 16u;

                check(w[{0, 3}]) == std::pair{0L, 3L};
                check(calls) == 16u;

                check(w[{5, 7}]) == std::pair(5L, 7L);
                check(calls) == 32u;

                check(w[{345, 127}]) == std::pair(345L, 127L);
                check(calls) == 48u;

                auto pos = w.chunks();
                check(pos.next()->first) == planet::map::coordinates{0, 0};
                check(pos.next()->first) == planet::map::coordinates{4, 4};
                check(pos.next()->first) == planet::map::coordinates{344, 124};
                check(pos.next()).is_falsey();
            },
            [](auto check) {
                std::size_t cell_number{};
                planet::map::world_type<std::size_t, 8> numbers{
                        {}, [&](auto) { return cell_number++; }};
                check(cell_number) == 0u;

                felspar::coro::eager<> on_chunk_created;
                on_chunk_created.post(
                        +[](planet::map::world_type<std::size_t, 8> &n)
                                -> felspar::coro::eager<>::task_type {
                            co_await n.on_chunk_created.next();
                        },
                        std::ref(numbers));

                numbers[{0, 0}];
                check(cell_number) == 64u;
                std::move(on_chunk_created).release().get();
            });


}
