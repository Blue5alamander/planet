#include <planet/map.hpp>


namespace planet::hexmap {


    template<typename Cell, std::size_t DimX, std::size_t DimY = DimX / 2>
    using chunk = map::chunk<Cell, DimX, DimY>;


    /// ## Hex co-ordinates
    /**
     * These are stored with 2 hex rows occupying a single row in the
     * rectilinear co-ordinate space. The hex grid is aligned with points up.
     * The hex row on odd rows is aligned one unit to the side of the even rows.
     * This allows us to interleave the rows on odd/even pairs of the hex grid
     * into a single row of the rectilinear grid. As a consequence both
     * co-ordinate axes must be either odd or even.
     */
    class coordinate {
        map::coordinate pos;
        constexpr coordinate(map::coordinate p) : pos{p} {}

      public:
        constexpr coordinate() noexcept {}
        constexpr coordinate(long x, long y) noexcept
        : pos{x, (y < 0 ? y - 1 : y) / 2} {}
        /// Create a hex co-ordinate from the compressed co-ordinates
        static constexpr coordinate from_compressed(map::coordinate const p) {
            return {p};
        }

        /// Return the compressed co-ordinates
        constexpr map::coordinate compressed() const noexcept { return pos; }

        constexpr long row() const noexcept {
            return (pos.row() * 2) + (pos.column() bitand 1);
        }
        constexpr long column() const noexcept { return pos.column(); }

        constexpr coordinate operator+(coordinate const r) const noexcept {
            return {column() + r.column(), row() + r.row()};
        }

        constexpr auto operator<=>(coordinate const &) const noexcept = default;
    };

    constexpr coordinate east{2, 0}, north_east{1, 1}, north_west{-1, 1},
            west{-2, 0}, south_west{-1, -1}, south_east{1, -1};
    constexpr std::array<coordinate, 6> directions{
            east, north_east, north_west, west, south_west, south_east};


    template<typename Chunk>
    class world {
        static_assert(
                (Chunk::width bitand 1) == 0,
                "Width of chunks storage must be even");
        map::world<Chunk> grid;

      public:
        using chunk_type = Chunk;
        using cell_type = typename chunk_type::cell_type;
        using init_function_type = std::function<cell_type(coordinate)>;

        felspar::coro::generator<std::pair<coordinate, chunk_type *>> chunks() {
            for (auto c : grid.chunks()) {
                co_yield {coordinate::from_compressed(c.first), c.second};
            }
        }

        world(coordinate const start, init_function_type const ift)
        : grid{start.compressed(), [f = std::move(ift)](map::coordinate p) {
                   return f(coordinate::from_compressed(p));
               }} {}

        cell_type &operator[](coordinate p) { return grid[p.compressed()]; }
    };


}
