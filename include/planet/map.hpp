#pragma once


#include <planet/to_string.hpp>

#include <felspar/coro/generator.hpp>

#include <array>
#include <functional>
#include <memory>
#include <vector>


namespace planet::map {


    template<typename Cell, std::size_t DimX, std::size_t DimY = DimX>
    class chunk {
        std::array<Cell, DimX * DimY> storage;

      public:
        using cell_type = Cell;
        static constexpr std::size_t width = DimX, height = DimY;

        template<typename Init>
        constexpr chunk(Init cell) {
            for (std::size_t x{}; x < width; ++x) {
                for (std::size_t y{}; y < height; ++y) {
                    (*this)[{x, y}] = cell(x, y);
                }
            }
        }

        constexpr Cell &operator[](std::pair<std::size_t, std::size_t> const p) {
            return storage.at(p.first * height + p.second);
        }
    };


    /// ## Cell & Super-cell Co-ordinates
    /**
     * Directions when looking at the map
     * x-axis is right to left -- increases left
     * y-axis is bottom to top -- increases up
     */
    class coordinates {
        long x = {}, y = {};

      public:
        constexpr coordinates() noexcept {}
        constexpr coordinates(long x, long y) noexcept : x{x}, y{y} {}

        constexpr auto row() const noexcept { return y; }
        constexpr auto column() const noexcept { return x; }

        constexpr coordinates operator+(coordinates const r) const noexcept {
            return {x + r.x, y + r.y};
        }

        constexpr auto operator<=>(coordinates const &) const noexcept = default;

        /// The square of the magnitude of the location from the origin
        long mag2() const noexcept { return x * x + y * y; }

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

        mutable long bottom_edge = {};
        mutable std::vector<row> rows;

        mutable std::vector<std::pair<coordinates, std::unique_ptr<Chunk>>>
                storage;

        auto *cell_at(coordinates const p) const {
            auto const rows_inserted = coordinates::insert_count(
                    bottom_edge, p.row(), chunk_type::height);
            rows.insert(rows.begin(), rows_inserted, row{});
            bottom_edge -= chunk_type::height * rows_inserted;

            std::size_t const row_number = coordinates::chunk_number(
                    bottom_edge, p.row(), chunk_type::height);
            if (rows.size() <= row_number) {
                rows.resize(row_number + 1, row{});
            }
            auto &row = rows[row_number];

            auto const cols_inserted = coordinates::insert_count(
                    row.left_edge, p.column(), chunk_type::width);
            row.chunks.insert(row.chunks.begin(), cols_inserted, nullptr);
            row.left_edge -= cols_inserted * chunk_type::width;

            std::size_t const cell_number = coordinates::chunk_number(
                    row.left_edge, p.column(), chunk_type::width);
            if (row.chunks.size() <= cell_number) {
                row.chunks.resize(cell_number + 1, nullptr);
            }
            auto &chunk = row.chunks[cell_number];

            if (chunk == nullptr) {
                auto const offx = cell_number * chunk_type::width;
                auto const offy = row_number * chunk_type::height;
                storage.emplace_back(std::pair{
                        coordinates{
                                row.left_edge + long(offx),
                                bottom_edge + long(offy)},
                        std::make_unique<chunk_type>(
                                [=, this](auto const x, auto const y) {
                                    auto const relx = offx + x;
                                    auto const rely = offy + y;
                                    return init(
                                            {row.left_edge + long(relx),
                                             bottom_edge + long(rely)});
                                })});
                chunk = storage.back().second.get();
            }

            auto const inx = coordinates::inside_chunk(
                    row.left_edge, p.column(), chunk_type::width);
            auto const iny = coordinates::inside_chunk(
                    bottom_edge, p.row(), chunk_type::height);

            return &(*chunk)[{inx, iny}];
        }

      public:
        using chunk_type = Chunk;
        using cell_type = typename chunk_type::cell_type;
        using init_function_type = std::function<cell_type(coordinates)>;

        world(coordinates const start, init_function_type const ift)
        : bottom_edge{start.row()}, rows{row{start.column()}}, init{ift} {}

        felspar::coro::generator<std::pair<coordinates, chunk_type *>> chunks() {
            for (auto &c : storage) { co_yield {c.first, c.second.get()}; }
        }

        cell_type &operator[](coordinates const p) { return *cell_at(p); }
        cell_type const &operator[](coordinates const p) const {
            return *cell_at(p);
        }

      private:
        init_function_type init;
    };


}
