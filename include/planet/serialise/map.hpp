#pragma once


#include <planet/map.hpp>
#include <planet/serialise/save_buffer.hpp>


namespace planet::map {


    /// ### `planet::map::chunk`


    template<typename Cell, std::size_t DimX, std::size_t DimY>
    inline serialise::save_buffer &
            save(serialise::save_buffer &ab, chunk<Cell, DimX, DimY> const &c) {
        return ab.save_box("_p:m:chunk", c.storage);
    }
    template<typename Cell, std::size_t DimX, std::size_t DimY>
    inline void load(std::span<std::byte const> &s, chunk<Cell, DimX, DimY> &c) {
        return serialise::load_box(s, "_p:m:chunk", c.storage);
    }

    /// ### `planet::map::world`


    template<typename Chunk>
    inline serialise::save_buffer &save(
            serialise::save_buffer &ab,
            std::vector<std::pair<coordinates, std::unique_ptr<Chunk>>> const
                    &v) {
        /**
         * The default saver for this vector will produce output that will be
         * harder for the specialised loader we need to load the items back in.
         * So just keep things simple.
         */
        ab.append(v.size());
        for (auto const &p : v) {
            save(ab, p.first);
            save(ab, *p.second);
        }
        return ab;
    }
    template<typename Chunk>
    inline serialise::save_buffer &
            save(serialise::save_buffer &ab, world<Chunk> const &w) {
        return ab.save_box("_p:m:world", w.storage);
    }
    template<typename Chunk>
    inline void load(std::span<std::byte const> &l, world<Chunk> &w) {
        /**
         * Because we don't have a default constructor for the chunk, and the
         * internal bookkeeping that we need in the world is a bit complicated
         * we have to load in a different way as well.
         */
        auto box = serialise::load_type<serialise::box>(l);
        box.check_name_or_throw("_p:m:world");
        auto const items = serialise::load_type<std::size_t>(l);
        for (std::size_t index{}; index < items; ++index) {
            auto const pos = serialise::load_type<coordinates>(l);
            auto &chunk = w[pos];
            load(box.content, chunk);
        }
        box.check_empty_or_throw();
    }


}


namespace planet::hexmap {


    /// ### `planet::hexmap::world`


    template<typename Chunk>
    inline serialise::save_buffer &
            save(serialise::save_buffer &ab, world<Chunk> const &w) {
        return ab.save_box("_p:h:world", w.grid);
    }
    template<typename Chunk>
    inline void load(std::span<std::byte const> &s, world<Chunk> &w) {
        return serialise::load_box(s, "_p:h:world", w.grid);
    }


}
