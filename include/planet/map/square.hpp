#pragma once


#include <planet/serialise/forward.hpp>
#include <planet/to_string.hpp>

#include <felspar/coro/bus.hpp>
#include <felspar/coro/generator.hpp>

#include <array>
#include <compare>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <vector>


namespace planet::hexmap {
    class coordinates;
}


namespace planet::map {


    /// ### Chunk
    /**
     * The map is split up into rectangular chunks. The `Cell` type is what is
     * stored at each location on the map.
     */
    template<typename Cell, std::size_t DimX, std::size_t DimY = DimX>
    class chunk {
        std::array<Cell, DimX * DimY> storage;

      public:
        using cell_type = Cell;
        static constexpr std::size_t width = DimX, height = DimY;


        /// ### Construction
        template<typename Init>
        explicit constexpr chunk(Init cell) {
            for (std::size_t x{}; x < width; ++x) {
                for (std::size_t y{}; y < height; ++y) {
                    (*this)[{x, y}] = cell(x, y);
                }
            }
        }


        /// ### Access into the cells within the chunk
        constexpr Cell &operator[](std::pair<std::size_t, std::size_t> const p) {
            return storage.at(p.first * height + p.second);
        }
        std::span<Cell, DimX * DimY> cells() noexcept { return storage; }
        std::span<Cell const, DimX * DimY> cells() const noexcept {
            return storage;
        }


        /// ### Serialise
        template<typename C, std::size_t X, std::size_t Y>
        friend void save(serialise::save_buffer &, chunk<C, X, Y> const &);
        template<typename C, std::size_t X, std::size_t Y>
        friend void load(serialise::load_buffer &, chunk<C, X, Y> &);
    };


    /// ## Cell & Super-cell Co-ordinates
    /**
     * Directions when looking at the map:
     * - x-axis is right to left -- increases left
     * - y-axis is bottom to top -- increases up
     */
    class coordinates {
        friend class hexmap::coordinates;

      public:
        using value_type = std::int32_t;

        constexpr coordinates() noexcept {}
        constexpr coordinates(value_type x, value_type y) noexcept
        : x{x}, y{y} {}

        constexpr auto row() const noexcept { return y; }
        constexpr auto column() const noexcept { return x; }

        constexpr coordinates operator+(coordinates const r) const noexcept {
            return {x + r.x, y + r.y};
        }

        constexpr auto operator<=>(coordinates const &) const noexcept = default;

        /// The square of the magnitude of the location from the origin
        value_type mag2() const noexcept { return x * x + y * y; }

        static constexpr std::size_t insert_count(
                value_type const lowest,
                value_type const position,
                std::size_t const width) noexcept {
            if (position < lowest) {
                return (lowest - position) / width + 1;
            } else {
                return {};
            }
        }
        static constexpr std::size_t chunk_number(
                value_type const lowest,
                value_type const position,
                std::size_t const width) noexcept {
            return (position - lowest) / width;
        }
        static constexpr std::size_t inside_chunk(
                value_type const lowest,
                value_type const position,
                std::size_t const width) noexcept {
            return (position - lowest) % width;
        }

        /// ### Serialisation
        friend void save(serialise::save_buffer &, coordinates);
        friend void load(serialise::load_buffer &, coordinates &);

      private:
        value_type x = {}, y = {};
    };
    void save(serialise::save_buffer &, coordinates);
    void load(serialise::load_buffer &, coordinates &);


    /// ## The world map
    template<typename Chunk>
    class world {
        struct row {
            coordinates::value_type left_edge = {};
            std::vector<Chunk *> chunks = {};
        };

        mutable coordinates::value_type bottom_edge = {};
        mutable std::vector<row> rows;

        mutable std::vector<std::pair<coordinates, std::unique_ptr<Chunk>>>
                storage;

        std::pair<Chunk *, row const *> chunk_ptr(coordinates const p) const {
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
                                row.left_edge + coordinates::value_type(offx),
                                bottom_edge + coordinates::value_type(offy)},
                        std::make_unique<chunk_type>([=,
                                                      this](auto const x,
                                                            auto const y) {
                            auto const relx = offx + x;
                            auto const rely = offy + y;
                            return init(
                                    {row.left_edge
                                             + coordinates::value_type(relx),
                                     bottom_edge
                                             + coordinates::value_type(rely)});
                        })});
                chunk = storage.back().second.get();
                on_chunk_created.push({storage.back().first, chunk});
            }
            return {chunk, &row};
        }
        auto *cell_at(coordinates const p) const {
            auto const [cp, rp] = chunk_ptr(p);
            auto const inx = coordinates::inside_chunk(
                    rp->left_edge, p.column(), chunk_type::width);
            auto const iny = coordinates::inside_chunk(
                    bottom_edge, p.row(), chunk_type::height);
            return &(*cp)[{inx, iny}];
        }

      public:
        using chunk_type = Chunk;
        using cell_type = typename chunk_type::cell_type;
        using init_function_type = std::function<cell_type(coordinates)>;
        static constexpr std::size_t chunk_width = Chunk::width,
                                     chunk_height = Chunk::height;


        /// ### Construction
        world() {}
        world(coordinates const start)
        : bottom_edge{start.row()}, rows{row{start.column()}} {}
        world(coordinates const start, init_function_type const ift)
        : world{start} {
            init = std::move(ift);
        }


        /// ### Access into chunks
        std::size_t chunk_count() const noexcept { return storage.size(); }
        using chunk_position = std::pair<coordinates, chunk_type *>;
        felspar::coro::generator<chunk_position> chunks() {
            for (std::size_t i{}; i < storage.size(); ++i) {
                auto &c = storage[i];
                co_yield {c.first, c.second.get()};
            }
        }
        auto &chunk_at(coordinates const p) { return *chunk_ptr(p).first; }
        auto const &chunk_at(coordinates const p) const {
            return *chunk_ptr(p).first;
        }
        mutable felspar::coro::bus<chunk_position> on_chunk_created;


        /// ### Access to cells
        cell_type &operator[](coordinates const p) { return *cell_at(p); }
        cell_type const &operator[](coordinates const p) const {
            return *cell_at(p);
        }


        /// ### Serialise
        template<typename C>
        friend void save(serialise::save_buffer &, world<C> const &);
        template<typename C>
        friend void load(serialise::load_buffer &, world<C> &);

      private:
        init_function_type init{[](coordinates) { return cell_type{}; }};
    };

    template<typename C, std::size_t X, std::size_t Y = X>
    using world_type = world<chunk<C, X, Y>>;


}
