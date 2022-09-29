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


    template<typename Chunk>
    class supercell {};



    /// ## Cell & Super-cell Co-ordinates
    /**
     * Directions when looking at the map
     * x-axis is right to left -- increases left
     * y-axis is bottom to top -- increases up
     */
    class coordinate {
        long x = {}, y = {};

      public:
        coordinate() noexcept {}
        coordinate(long x, long y) noexcept : x{x}, y{y} {}

        auto row() const noexcept { return y; }
        auto column() const noexcept { return x; }

        coordinate e() const noexcept { return {x + 2, y}; }
        coordinate ne() const noexcept { return {x + 1, y + 1}; }
        coordinate nw() const noexcept { return {x - 1, y + 1}; }
        coordinate w() const noexcept { return {x - 2, y}; }
        coordinate sw() const noexcept { return {x - 1, y - 1}; }
        coordinate se() const noexcept { return {x + 1, y - 1}; }
    };


    template<typename Cell>
    class world {
        std::vector<std::unique_ptr<Cell>> chunks;

      public:
    };


}
