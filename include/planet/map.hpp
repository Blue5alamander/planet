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


    class coordinate {
      public:
        coordinate(long x, long y);
    };


    template<typename Cell>
    class world {
        std::vector<std::unique_ptr<Cell>> chunks;

      public:
    };


}
