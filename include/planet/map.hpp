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

        static constexpr std::size_t insert_count(
                long const lowest,
                long const position,
                std::size_t const width) noexcept {
            if (position < lowest) {
                return (lowest - position) / width + 1;
            } else {
                return {};
            }
        }
        static constexpr std::size_t chunk_number(
                long const lowest,
                long const position,
                std::size_t const width) noexcept {
            return (position - lowest) / width;
        }
        static constexpr std::size_t inside_chunk(
                long const lowest,
                long const position,
                std::size_t const width) noexcept {
            return (position - lowest) % width;
        }
    };


    /// ## The world map
    template<typename Chunk>
    class world {
        struct row {
            long left_edge = {};
            std::vector<Chunk *> chunks;
        };

        long bottom_edge;
        std::vector<row> rows;

        std::vector<std::pair<coordinate, std::unique_ptr<Chunk>>> storage;

      public:
        using chunk_type = Chunk;
        using cell_type = typename chunk_type::cell_type;
        using init_function_type = std::function<cell_type(coordinate)>;

        world(coordinate const start, init_function_type const ift)
        : bottom_edge{start.row()}, rows{row{start.column()}}, init{ift} {}

        auto begin() noexcept { return storage.begin(); }
        auto end() noexcept { return storage.end(); }

        cell_type &operator[](std::pair<long, long> const p) {
            auto const rows_inserted = coordinate::insert_count(
                    bottom_edge, p.second, chunk_type::height);
            rows.insert(rows.begin(), rows_inserted, row{});
            bottom_edge -= chunk_type::height * rows_inserted;

            std::size_t const row_number = coordinate::chunk_number(
                    bottom_edge, p.second, chunk_type::height);
            if (rows.size() <= row_number) {
                rows.resize(row_number + 1, row{});
            }
            auto &row = rows[row_number];

            auto const cols_inserted = coordinate::insert_count(
                    row.left_edge, p.first, chunk_type::width);
            row.chunks.insert(row.chunks.begin(), cols_inserted, nullptr);
            row.left_edge -= cols_inserted * chunk_type::width;

            std::size_t const cell_number = coordinate::chunk_number(
                    row.left_edge, p.first, chunk_type::width);
            if (row.chunks.size() <= cell_number) {
                row.chunks.resize(cell_number + 1, nullptr);
            }
            auto &chunk = row.chunks[cell_number];

            auto const inx = coordinate::inside_chunk(
                    row.left_edge, p.first, chunk_type::width);
            auto const iny = coordinate::inside_chunk(
                    bottom_edge, p.second, chunk_type::height);

            if (chunk == nullptr) {
                auto const offx = cell_number * chunk_type::width;
                auto const offy = row_number * chunk_type::height;
                storage.emplace_back(std::pair{
                        coordinate{
                                row.left_edge + long(offx),
                                bottom_edge + long(offy)},
                        std::make_unique<chunk_type>(
                                [&](auto const x, auto const y) {
                                    auto const relx = offx + x;
                                    auto const rely = offy + y;
                                    return init(
                                            {row.left_edge + long(relx),
                                             bottom_edge + long(rely)});
                                })});
                chunk = storage.back().second.get();
            }

            return (*chunk)[{inx, iny}];
        }

      private:
        init_function_type init;
    };


}
