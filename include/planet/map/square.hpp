#pragma once


#include <planet/map/forward.hpp>
#include <planet/serialise/forward.hpp>
#include <planet/to_string.hpp>

#include <felspar/coro/bus.hpp>
#include <felspar/coro/generator.hpp>

#include <array>
#include <compare>
#include <concepts>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <span>
#include <type_traits>
#include <vector>


namespace planet::map::square {


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
            requires std::invocable<Init &, std::size_t, std::size_t>
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
        constexpr Cell const &
                operator[](std::pair<std::size_t, std::size_t> const p) const {
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
     * - x-axis is right to left -- increases to the left
     * - y-axis is bottom to top -- increases upwards
     *
     * The coordinates are 32 bit integers, so have an effective range of
     * around ±2 billion.
     */
    class coordinates {
        friend class hex::coordinates;

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

        /// #### The square of the magnitude of the location from the origin
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
    /**
     * The `Pointer` template parameter chooses how the world owns its chunks:
     *
     * - `std::unique_ptr` (the default) -- the world is the sole owner of its
     *   chunks. It cannot be copied and its cells are freely mutable.
     * - `std::shared_ptr` -- the world can be copied, with all chunks shared
     *   between the copies, and it only ever hands out `const` cells and
     *   chunks when read. This allows the world to be used as a persistent
     *   data structure where each edit produces a new version of the world.
     *
     * To produce a new version of a shared world, take a copy and then
     * `alter` cells in the copy, passing the world the copy was made from as
     * the `base`:
     *
     * ```cpp
     * auto next = current;
     * next.alter(current, position) = new_cell_value;
     * ```
     *
     * Taking the copy is cheap because no chunks are copied by it. The first
     * `alter` within any given chunk copies that chunk, so the change is only
     * visible in `next` -- `current`, and any older versions of the world,
     * still see the original cell content. Every other chunk remains shared
     * between the two worlds, and later `alter`s within an already copied
     * chunk see that it differs from the one `base` holds and edit it
     * directly.
     *
     * A brand new world (one that wasn't copied from anywhere) can be
     * populated by passing the world itself as the `base`, in which case
     * `alter` never copies and edits the cells in place.
     */
    template<
            typename Chunk,
            template<typename...> typename Pointer = std::unique_ptr>
    class world {
      public:
        using chunk_type = Chunk;
        using cell_type = typename chunk_type::cell_type;
        using pointer_type = Pointer<Chunk>;
        using init_function_type = std::function<cell_type(coordinates)>;
        static constexpr std::size_t chunk_width = Chunk::width,
                                     chunk_height = Chunk::height;
        /// ### True when chunks are shared between copies of the world
        static constexpr bool shared_chunks =
                std::is_same_v<pointer_type, std::shared_ptr<Chunk>>;
        static_assert(
                shared_chunks
                        or std::is_same_v<pointer_type, std::unique_ptr<Chunk>>,
                "Chunks must be held by either std::unique_ptr or "
                "std::shared_ptr");

      private:
        struct row {
            coordinates::value_type left_edge = {};
            /// Positions of the row's chunks within `storage`
            std::vector<std::size_t> chunks = {};
        };
        static constexpr std::size_t no_chunk =
                std::numeric_limits<std::size_t>::max();

        mutable coordinates::value_type bottom_edge = {};
        mutable std::vector<row> rows;

        mutable std::vector<std::pair<coordinates, pointer_type>> storage;

        template<typename Init>
        static pointer_type make_chunk(Init init) {
            if constexpr (shared_chunks) {
                return std::make_shared<chunk_type>(std::move(init));
            } else {
                return std::make_unique<chunk_type>(std::move(init));
            }
        }

        std::pair<std::size_t, row const *>
                chunk_index(coordinates const p) const {
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
            row.chunks.insert(row.chunks.begin(), cols_inserted, no_chunk);
            row.left_edge -= cols_inserted * chunk_type::width;

            std::size_t const cell_number = coordinates::chunk_number(
                    row.left_edge, p.column(), chunk_type::width);
            if (row.chunks.size() <= cell_number) {
                row.chunks.resize(cell_number + 1, no_chunk);
            }
            auto &index = row.chunks[cell_number];

            if (index == no_chunk) {
                auto const offx = cell_number * chunk_type::width;
                auto const offy = row_number * chunk_type::height;
                storage.emplace_back(
                        std::pair{
                                coordinates{
                                        row.left_edge
                                                + coordinates::value_type(offx),
                                        bottom_edge
                                                + coordinates::value_type(offy)},
                                make_chunk([=,
                                            this](auto const x, auto const y) {
                                    auto const relx = offx + x;
                                    auto const rely = offy + y;
                                    return init(
                                            {row.left_edge
                                                     + coordinates::value_type(
                                                             relx),
                                             bottom_edge
                                                     + coordinates::value_type(
                                                             rely)});
                                })});
                index = storage.size() - 1;
                on_chunk_created.push(
                        {storage.back().first, storage.back().second.get()});
            }
            return {index, &row};
        }
        std::pair<std::size_t, std::size_t> cell_location(
                row const *const rp, coordinates const p) const noexcept {
            return {coordinates::inside_chunk(
                            rp->left_edge, p.column(), chunk_type::width),
                    coordinates::inside_chunk(
                            bottom_edge, p.row(), chunk_type::height)};
        }
        /**
         * Return the chunk covering the position, or `nullptr` when the world
         * hasn't created a chunk there. Never creates the chunk.
         */
        chunk_type const *find_chunk(coordinates const p) const noexcept {
            if (coordinates::insert_count(
                        bottom_edge, p.row(), chunk_type::height)
                > 0) {
                return nullptr;
            }
            std::size_t const row_number = coordinates::chunk_number(
                    bottom_edge, p.row(), chunk_type::height);
            if (rows.size() <= row_number) { return nullptr; }
            auto const &row = rows[row_number];
            if (coordinates::insert_count(
                        row.left_edge, p.column(), chunk_type::width)
                > 0) {
                return nullptr;
            }
            std::size_t const cell_number = coordinates::chunk_number(
                    row.left_edge, p.column(), chunk_type::width);
            if (row.chunks.size() <= cell_number) { return nullptr; }
            auto const index = row.chunks[cell_number];
            if (index == no_chunk) { return nullptr; }
            return storage[index].second.get();
        }

      public:
        /// ### Construction
        world() {}
        world(coordinates const start)
        : bottom_edge{start.row()}, rows{row{start.column()}} {}
        world(coordinates const start, init_function_type ift) : world{start} {
            init = std::move(ift);
        }

        /// ### Copying and moving
        /**
         * Only worlds with shared chunks can be copied. The copy shares every
         * chunk with the original, and a chunk is only copied when it is
         * first edited through `alter`. The copy starts with an
         * `on_chunk_created` bus that has no subscribers, and copy assignment
         * leaves the target's bus alone.
         */
        world(world const &w)
            requires shared_chunks
        : bottom_edge{w.bottom_edge},
          rows{w.rows},
          storage{w.storage},
          init{w.init} {}
        world &operator=(world const &w)
            requires shared_chunks
        {
            bottom_edge = w.bottom_edge;
            rows = w.rows;
            storage = w.storage;
            init = w.init;
            return *this;
        }
        world(world &&) = default;
        world &operator=(world &&) = default;


        /// ### Access into chunks
        /**
         * Worlds with shared chunks only allow `const` access so that a chunk
         * shared with another world can never be changed by accident.
         */
        std::size_t chunk_count() const noexcept { return storage.size(); }
        using chunk_position = std::pair<coordinates, chunk_type *>;
        using const_chunk_position = std::pair<coordinates, chunk_type const *>;
        felspar::coro::generator<chunk_position> chunks()
            requires(not shared_chunks)
        {
            for (std::size_t i{}; i < storage.size(); ++i) {
                auto &c = storage[i];
                co_yield {c.first, c.second.get()};
            }
        }
        felspar::coro::generator<const_chunk_position> chunks() const {
            for (std::size_t i{}; i < storage.size(); ++i) {
                auto &c = storage[i];
                co_yield {c.first, c.second.get()};
            }
        }
        chunk_type &chunk_at(coordinates const p)
            requires(not shared_chunks)
        {
            return *storage[chunk_index(p).first].second;
        }
        chunk_type const &chunk_at(coordinates const p) const {
            return *storage[chunk_index(p).first].second;
        }
        mutable felspar::coro::bus<std::conditional_t<
                shared_chunks,
                const_chunk_position,
                chunk_position>>
                on_chunk_created;


        /// ### Access to cells
        cell_type &operator[](coordinates const p)
            requires(not shared_chunks)
        {
            auto const [index, rp] = chunk_index(p);
            auto const location = cell_location(rp, p);
            return (*storage[index].second)[location];
        }
        cell_type const &operator[](coordinates const p) const {
            auto const [index, rp] = chunk_index(p);
            auto const location = cell_location(rp, p);
            return (*storage[index].second)[location];
        }


        /// ### Alter a cell
        /**
         * Return a mutable cell so that its content can be changed. The world
         * is treated as an edit of `base` -- typically the world that this
         * one was copied from. If the chunk holding the cell is still shared
         * with `base` then it is copied first, so a change made through the
         * returned reference is never observable through `base`. Once the
         * chunk already differs from the one `base` holds -- including when
         * `base` is this world, or when `base` never created the chunk -- the
         * cell is returned without any copying.
         */
        cell_type &alter(world const &base, coordinates const p)
            requires shared_chunks
        {
            auto const [index, rp] = chunk_index(p);
            auto &pointer = storage[index].second;
            if (&base != this and pointer.get() == base.find_chunk(p)) {
                pointer = std::make_shared<chunk_type>(*pointer);
            }
            auto const location = cell_location(rp, p);
            return (*pointer)[location];
        }


        /// ### Serialise
        template<typename C, template<typename...> typename P>
        friend void save(serialise::save_buffer &, world<C, P> const &);
        template<typename C, template<typename...> typename P>
        friend void load(serialise::load_buffer &, world<C, P> &);

      private:
        init_function_type init{[](coordinates) { return cell_type{}; }};
    };

    template<
            typename C,
            std::size_t X,
            std::size_t Y = X,
            template<typename...> typename Pointer = std::unique_ptr>
    using world_type = world<chunk<C, X, Y>, Pointer>;


}
