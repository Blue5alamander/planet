#include <planet/map.hpp>
#include <felspar/test.hpp>

#include <felspar/coro/eager.hpp>

#include <concepts>


namespace {


    auto const chunk = felspar::testsuite("map::square/chunk", [](auto check) {
        planet::map::square::chunk<std::pair<std::size_t, std::size_t>, 2> c{
                [](auto x, auto y) { return std::pair{x, y}; }};

        check(c[{0, 0}]) == std::pair{std::size_t{}, std::size_t{}};
        check(c[{1, 0}]) == std::pair{std::size_t{1}, std::size_t{}};
        check(c[{0, 1}]) == std::pair{std::size_t{}, std::size_t{1}};
        check(c[{1, 1}]) == std::pair{std::size_t{1}, std::size_t{1}};
    });


    auto const coords = felspar::testsuite(
            "map::square/coordinates",
            [](auto check) {
                planet::map::square::coordinates loc{};
                check(loc.row()) == 0;
                check(loc.column()) == 0;
            },
            [](auto check) {
                check(planet::map::square::coordinates::insert_count(0, 4, 8))
                        == 0u;
                check(planet::map::square::coordinates::insert_count(4, 0, 8))
                        == 1u;
                check(planet::map::square::coordinates::insert_count(0, -4, 8))
                        == 1u;
                check(planet::map::square::coordinates::insert_count(0, -7, 8))
                        == 1u;
                check(planet::map::square::coordinates::insert_count(0, -8, 8))
                        == 2u;
            },
            [](auto check) {
                check(planet::map::square::coordinates::chunk_number(0, 0, 8))
                        == 0UL;
                check(planet::map::square::coordinates::chunk_number(-8, -4, 8))
                        == 0UL;
                check(planet::map::square::coordinates::chunk_number(-8, 0, 8))
                        == 1UL;
                check(planet::map::square::coordinates::chunk_number(-8, -1, 8))
                        == 0UL;
            },
            [](auto check) {
                check(planet::map::square::coordinates::inside_chunk(0, 4, 8))
                        == 4UL;
                check(planet::map::square::coordinates::inside_chunk(0, 7, 8))
                        == 7UL;
                check(planet::map::square::coordinates::inside_chunk(0, 8, 8))
                        == 0UL;
                check(planet::map::square::coordinates::inside_chunk(0, 14, 8))
                        == 6UL;
                check(planet::map::square::coordinates::inside_chunk(-8, -1, 8))
                        == 7UL;
                check(planet::map::square::coordinates::inside_chunk(-16, -1, 8))
                        == 7UL;
            },
            [](auto check) {
                check(planet::map::square::coordinates{3, 4}.mag2()) == 25;
            });


    auto const world = felspar::testsuite(
            "map::square/world",
            [](auto check) {
                std::size_t calls{};
                planet::map::square::world<
                        planet::map::square::chunk<std::pair<long, long>, 4>>
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
                check(pos.next()->first)
                        == planet::map::square::coordinates{0, 0};
                check(pos.next()->first)
                        == planet::map::square::coordinates{4, 4};
                check(pos.next()->first)
                        == planet::map::square::coordinates{344, 124};
                check(pos.next()).is_falsey();
            },
            [](auto check) {
                std::size_t cell_number{};
                planet::map::square::world_type<std::size_t, 8> numbers{
                        {}, [&](auto) { return cell_number++; }};
                check(cell_number) == 0u;

                felspar::coro::eager<> on_chunk_created;
                on_chunk_created.post(
                        +[](planet::map::square::world_type<std::size_t, 8> &n)
                                -> felspar::coro::eager<>::task_type {
                            co_await n.on_chunk_created.next();
                        },
                        std::ref(numbers));

                numbers[{0, 0}];
                check(cell_number) == 64u;
                std::move(on_chunk_created).release().get();
            },
            [](auto check) {
                planet::map::square::world<
                        planet::map::square::chunk<std::pair<long, long>, 4>>
                        w{{0, 0}, [](auto const p) {
                              return std::pair{p.column(), p.row()};
                          }};
                /**
                 * Nothing is created until a cell is touched, so there is
                 * nothing to walk yet.
                 */
                check(w.cells().next()).is_falsey();

                /// Touching the origin builds its 4x4 chunk.
                w[{0, 0}];

                /**
                 * `cells()` walks each chunk column-major (x outer, y inner)
                 * and pairs every cell with its world coordinate. The init made
                 * each cell equal to its own `{column, row}`.
                 */
                auto cells = w.cells();
                auto const first = cells.next();
                check(first->first) == planet::map::square::coordinates{0, 0};
                check(first->second) == std::pair{0L, 0L};
                auto const second = cells.next();
                check(second->first) == planet::map::square::coordinates{0, 1};
                check(second->second) == std::pair{0L, 1L};

                std::size_t count{2};
                while (cells.next()) { ++count; }
                check(count) == 16u;
            },
            [](auto check) {
                planet::map::square::world<
                        planet::map::square::chunk<std::pair<long, long>, 4>>
                        w{{0, 0}, [](auto const p) {
                              return std::pair{p.column(), p.row()};
                          }};
                w[{0, 0}];

                /**
                 * The mutable overload hands back a live reference into the
                 * world.
                 */
                w.cells().next()->second = std::pair{99L, 99L};
                check(w[{0, 0}]) == std::pair{99L, 99L};

                /// The `const` overload compiles and yields read-only cells.
                auto const &cw = w;
                std::size_t count{};
                for (auto const [location, cell] : cw.cells()) {
                    static_cast<void>(location);
                    static_cast<void>(cell);
                    ++count;
                }
                check(count) == 16u;
            });


    auto const shared = felspar::testsuite(
            "map::square/world/shared",
            [](auto check) {
                /**
                 * Shared worlds only ever hand out `const` access when read,
                 * so cells have to be changed through `alter`.
                 */
                using shared_world = planet::map::square::world_type<
                        long, 4, 4, std::shared_ptr>;
                static_assert(not requires(
                        shared_world &w,
                        planet::map::square::coordinates const p) {
                    { w[p] } -> std::same_as<long &>;
                });
                static_assert(requires(
                        shared_world &w,
                        planet::map::square::coordinates const p) {
                    { w[p] } -> std::same_as<long const &>;
                });

                /**
                 * A copied world shares all of its chunks with the original,
                 * and `alter` copies a chunk the first time it is edited.
                 */
                shared_world w{{0, 0}, [](auto) { return 0L; }};
                w.alter(w, {1, 1}) = 42;
                w.alter(w, {5, 5}) = 43;
                check(w.chunk_count()) == 2u;
                auto const &cw = w;
                check(cw[{1, 1}]) == 42L;

                auto next = w;
                auto const &cnext = next;
                check(next.chunk_count()) == 2u;
                check(&cnext.chunk_at({1, 1})) == &cw.chunk_at({1, 1});
                check(&cnext.chunk_at({5, 5})) == &cw.chunk_at({5, 5});

                next.alter(w, {1, 1}) = 99;
                check(cnext[{1, 1}]) == 99L;
                check(cw[{1, 1}]) == 42L;
                check(&cnext.chunk_at({1, 1})) != &cw.chunk_at({1, 1});
                /// The chunk that wasn't edited is still shared
                check(&cnext.chunk_at({5, 5})) == &cw.chunk_at({5, 5});
                check(cnext[{5, 5}]) == 43L;

                /// A second edit within the same chunk doesn't copy it again
                auto const *const already_copied = &cnext.chunk_at({1, 1});
                next.alter(w, {2, 2}) = 7;
                check(&cnext.chunk_at({1, 1})) == already_copied;
                check(cw[{2, 2}]) == 0L;
                check(cnext[{2, 2}]) == 7L;
            },
            [](auto check) {
                /// Altering a chunk that the base world never created
                planet::map::square::world_type<long, 4, 4, std::shared_ptr> w{
                        {0, 0}, [](auto) { return 0L; }};
                w.alter(w, {0, 0}) = 1;
                auto next = w;
                next.alter(w, {9, 9}) = 2;
                check(next.chunk_count()) == 2u;
                check(w.chunk_count()) == 1u;
            },
            [](auto check) {
                /// Copy assignment also shares chunks
                planet::map::square::world_type<long, 4, 4, std::shared_ptr> w{
                        {0, 0}, [](auto) { return 0L; }};
                w.alter(w, {0, 0}) = 7;
                planet::map::square::world_type<long, 4, 4, std::shared_ptr>
                        other;
                other = w;
                auto const &cw = w;
                auto const &cother = other;
                check(&cother.chunk_at({0, 0})) == &cw.chunk_at({0, 0});
                other.alter(w, {0, 0}) = 8;
                check(cw[{0, 0}]) == 7L;
                check(cother[{0, 0}]) == 8L;
            });


}
