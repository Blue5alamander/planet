#pragma once


#include <planet/affine2d.hpp>
#include <planet/map.hpp>

#include <numbers>


namespace planet::hexmap {


    template<typename Cell, std::size_t DimX, std::size_t DimY = DimX / 2>
    using chunk = map::chunk<Cell, DimX, DimY>;


    /// ## Hex co-ordinates
    /**
     * These are stored with 2 hex rows occupying a single row in the
     * rectilinear co-ordinate space. The hex grid is aligned with points up ⬡.
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

        /// Return the centre position of the hex
        constexpr point2d centre(float const r = 1.0f) const noexcept {
            /**
             * Given a hexagon, which is point up, with an inner radius `r`,
             * then the centre of the hex to the north-east of one centred on
             * the origin is at location `(r, h)`. We now that the two centres
             * are `2r` from each other, so by Pythagoras we can work out that
             * `h` must be `√3r`.
             */
            auto const h = std::numbers::sqrt3_v<float> * r;
            return {column() * r, row() * h};
        }
        /// Magnitude squared of the location from the origin
        constexpr float mag2(float const r = 1.0f) const noexcept {
            auto const p = centre(r);
            auto const x = p.x();
            auto const y = p.y();
            return x * x + y * y;
        }
        /// Return the 6 vertices for the hex, starting at the top going
        /// clockwise. The parameter `r` is used for the hex spacing, and `ir`
        /// is used as the radius for the vertices. If these values are
        /// different then the vertices will be either inside or outside of the
        /// vertex locations for a true tessellation.
        constexpr std::array<point2d, 6>
                vertices(float const r, float const ir) const noexcept {
            auto const c = centre(r);
            auto const h = std::numbers::sqrt3_v<float> * ir;
            return {
                    {c + point2d{0.0f, h}, c + point2d{ir, ir / 2},
                     c + point2d{ir, -ir / 2}, c + point2d{0.0f, -h},
                     c + point2d{-ir, -ir / 2}, c + point2d{-ir, ir / 2}}};
        }
        constexpr auto vertices(float const r = 1.0f) const noexcept {
            return vertices(r, r);
        }

        constexpr coordinates operator+(coordinates const r) const noexcept {
            return {column() + r.column(), row() + r.row()};
        }
        constexpr coordinates operator-(coordinates const r) const noexcept {
            return {column() - r.column(), row() - r.row()};
        }

        constexpr auto operator<=>(coordinates const &) const noexcept = default;

        /// Produce the co-ordinates iterating through columns first
        static felspar::coro::generator<coordinates>
                by_column(coordinates top_left, coordinates bottom_right) {
            auto const start_odd = top_left.row() bitand 1;
            for (auto row{top_left.row()}; row > bottom_right.row(); --row) {
                auto const row_odd = row bitand 1;
                for (auto col{top_left.column() + (start_odd xor row_odd)};
                     col < bottom_right.column(); col += 2) {
                    co_yield {col, row};
                }
            }
        }
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
        cell_type const &operator[](coordinates p) const {
            return grid[p.compressed()];
        }
    };


    template<typename C, std::size_t X, std::size_t Y = X / 2>
    using world_type = world<chunk<C, X, Y>>;


}
