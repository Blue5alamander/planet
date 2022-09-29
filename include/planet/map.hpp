#include <array>
#include <memory>
#include <vector>


namespace planet::map {


    template<typename Cell, std::size_t Dim>
    class chunk {
        std::array<Cell, Dim * Dim> storage;

      public:
        static constexpr std::size_t width = Dim, height = Dim;
    };


    template<typename Chunk, std::size_t Dim>
    class supercell {
        std::array<Chunk, Dim * Dim> storage;

      public:
        static constexpr std::size_t width = Dim * Chunk::width,
                                     height = Dim * Chunk::height;
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


    template<typename Cell>
    class world {
        std::vector<std::unique_ptr<Cell>> chunks;

      public:
    };


}
