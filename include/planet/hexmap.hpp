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
    class coordinates {
        map::coordinates pos;
        constexpr coordinates(map::coordinates p) : pos{p} {}

      public:
        constexpr coordinates() noexcept {}
        constexpr coordinates(long x, long y) noexcept
        : pos{x, (y < 0 ? y - 1 : y) / 2} {}
        /// Create a hex co-ordinate from the compressed co-ordinates
        static constexpr coordinates from_compressed(map::coordinates const p) {
            return {p};
        }

        /// Return the compressed co-ordinates
        constexpr map::coordinates compressed() const noexcept { return pos; }

        constexpr long row() const noexcept {
            return (pos.row() * 2) + (pos.column() bitand 1);
        }
        constexpr long column() const noexcept { return pos.column(); }

        constexpr coordinates operator+(coordinates const r) const noexcept {
            return {column() + r.column(), row() + r.row()};
        }

        constexpr auto operator<=>(coordinates const &) const noexcept = default;
    };

    inline std::string to_string(coordinates p) {
        return planet::to_string(std::pair{p.column(), p.row()});
    }

    constexpr coordinates east{2, 0}, north_east{1, 1}, north_west{-1, 1},
            west{-2, 0}, south_west{-1, -1}, south_east{1, -1};
    constexpr std::array<coordinates, 6> directions{
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
        using init_function_type = std::function<cell_type(coordinates)>;

        felspar::coro::generator<std::pair<coordinates, chunk_type *>> chunks() {
            for (auto c : grid.chunks()) {
                co_yield {coordinates::from_compressed(c.first), c.second};
            }
        }

        world(coordinates const start, init_function_type const ift)
        : grid{start.compressed(), [f = std::move(ift)](map::coordinates p) {
                   return f(coordinates::from_compressed(p));
               }} {}

        cell_type &operator[](coordinates p) { return grid[p.compressed()]; }
    };


    template<typename C, std::size_t X, std::size_t Y = X / 2>
    using world_type = world<chunk<C, X, Y>>;


}
