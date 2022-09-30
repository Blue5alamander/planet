#include <array>
#include <functional>
#include <memory>
#include <vector>


namespace planet::map {


    template<typename Cell, std::size_t Dim>
    class chunk {
        std::array<Cell, Dim * Dim> storage;

      public:
        using cell_type = Cell;
        static constexpr std::size_t width = Dim, height = Dim;

        template<typename Init>
        constexpr chunk(Init cell) {
            for (std::size_t x{}; x < width; ++x) {
                for (std::size_t y{}; y < height; ++y) {
                    (*this)[{x, y}] = cell(x, y);
                }
            }
        }

        constexpr Cell &operator[](std::pair<std::size_t, std::size_t> const p) {
            return storage[p.first * width + p.second];
        }
    };


    /// ## Cell & Super-cell Co-ordinates
    /**
     * Directions when looking at the map
     * x-axis is right to left -- increases left
     * y-axis is bottom to top -- increases up
     */
    class coordinate {
        long x = {}, y = {};

      public:
        constexpr coordinate() noexcept {}
        constexpr coordinate(long x, long y) noexcept : x{x}, y{y} {}

        constexpr auto row() const noexcept { return y; }
        constexpr auto column() const noexcept { return x; }

        constexpr coordinate operator+(coordinate const r) const noexcept {
            return {x + r.x, y + r.y};
        }

        constexpr auto operator<=>(coordinate const &) const noexcept = default;
    };

    constexpr coordinate east{2, 0}, north_east{1, 1}, north_west{-1, 1},
            west{-2, 0}, south_west{-1, -1}, south_east{1, -1};
    constexpr std::array<coordinate, 6> directions{
            east, north_east, north_west, west, south_west, south_east};


    /// ## The world map
    template<typename Chunk>
    class world {
        struct row {
            long left_edge;
            std::vector<std::unique_ptr<Chunk>> cells();
        };

        long bottom_edge;
        std::vector<row> columns;

      public:
        using chunk_type = Chunk;
        using cell_type = typename chunk_type::cell_type;
        using init_function_type = std::function<cell_type(coordinate)>;

        world(coordinate const start, init_function_type const ift)
        : bottom_edge{start.row()}, columns{row{start.column()}}, init{ift} {}

      private:
        init_function_type init;
    };


}
