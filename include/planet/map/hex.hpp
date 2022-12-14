#pragma once


#include <planet/affine2d.hpp>
#include <planet/map/square.hpp>
#include <planet/numbers.hpp>


namespace planet::hexmap {


    template<typename Cell, std::size_t DimX, std::size_t DimY = DimX / 2>
    using chunk = map::chunk<Cell, DimX, DimY>;


    /// Generate the 6 vertices for a hex at the specified location
    inline constexpr std::array<affine::point2d, 6>
            vertices(affine::point2d const c, float const ir) noexcept {
        auto const oR = 2.0f * ir / sqrt3;
        return {
                {c + affine::point2d{ir, oR / 2.0f},
                 c + affine::point2d{0.0f, oR},
                 c + affine::point2d{-ir, oR / 2.0f},
                 c + affine::point2d{-ir, -oR / 2.0f},
                 c + affine::point2d{0.0f, -oR},
                 c + affine::point2d{ir, -oR / 2.0f}}};
    }
    /// Return the bounding box for a hex of a given size at a given position
    inline constexpr affine::rectangle
            bounds(affine::point2d const c, float const ir) noexcept {
        auto const oR = 2.0f * ir / sqrt3;
        return {c + affine::point2d{-ir, -oR},
                affine::extents2d{2 * ir, 2 * oR}};
    }
    /// Return the distance a point is from the edges of a hex at the origin
    /// with the given inner radius. Adapted from
    /// <https://iquilezles.org/articles/distfunctions2d/>
    inline constexpr float
            offset_of(affine::point2d const c, float const ir = 1.0f) {
        /// kx is `-cos(pi/6)` and ky is `sin(pi/6)`. kr describes the out radius
        constexpr float kx{-0.8660254037844387f}, ky{0.5f}, kr{1.0f / sqrt3};

        /// Our hex is point up, so swap x & y
        float px{std::abs(c.y())}, py{std::abs(c.x())};

        auto const min_dot = std::min(0.0f, kx * px + ky * py);
        px -= 2.0f * min_dot * kx;
        py -= 2.0f * min_dot * ky;

        px -= std::clamp(px, -kr * ir, kr * ir);
        py -= ir;

        return std::copysign(std::sqrt(px * px + py * py), py);
    }


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
        constexpr affine::point2d centre(float const r = 1.0f) const noexcept {
            /**
             * Given a hexagon, which is point up, with an inner radius `r`,
             * then the centre of the hex to the north-east of one centred on
             * the origin is at location `(r, h)`. We now that the two centres
             * are `2r` from each other, so by Pythagoras we can work out that
             * `h` must be `√3r`.
             */
            auto const h = sqrt3 * r;
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
        constexpr std::array<affine::point2d, 6>
                vertices(float const r, float const ir) const noexcept {
            return hexmap::vertices(centre(r), ir);
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
        constexpr coordinates operator*(long const s) const noexcept {
            return {column() * s, row() * s};
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
