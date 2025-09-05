#pragma once


#include <planet/map.hpp>
#include <planet/serialise/save_buffer.hpp>


namespace planet::map::square {


    /// ## `planet::map::square::chunk`


    template<typename Cell, std::size_t DimX, std::size_t DimY>
    void save(serialise::save_buffer &ab, chunk<Cell, DimX, DimY> const &c) {
        ab.save_box("_p:m:chunk", c.storage);
    }
    template<typename Cell, std::size_t DimX, std::size_t DimY>
    void load(serialise::load_buffer &lb, chunk<Cell, DimX, DimY> &c) {
        lb.load_box("_p:m:chunk", c.storage);
    }


    /// ## `planet::map::square::world`


    template<typename Chunk>
    void save(
            serialise::save_buffer &ab,
            std::vector<std::pair<coordinates, std::unique_ptr<Chunk>>> const
                    &v) {
        /**
         * The default saver for this vector will produce output that will be
         * harder for the specialised loader we need to load the items back in.
         * So just keep things simple.
         */
        ab.append(serialise::marker::poly_list);
        ab.append_size_t(v.size() * 2);
        for (auto const &p : v) {
            save(ab, p.first);
            save(ab, *p.second);
        }
    }
    template<typename Chunk>
    void save(serialise::save_buffer &ab, world<Chunk> const &w) {
        ab.save_box("_p:m:world", w.storage);
    }
    template<typename Chunk>
    void load(serialise::load_buffer &lb, world<Chunk> &w) {
        /**
         * Because we don't have a default constructor for the chunk, and the
         * internal bookkeeping that we need in the world is a bit complicated
         * we have to load in a different way.
         *
         * The save file contains the position of the corner of each chunk, so
         * we can iterate through these and fetch the chunks at those positions
         * to load in -- this will overwrite the saved fields in the chunk's
         * data type and leave the others as if it was a fresh world.
         */
        auto box = serialise::expect_box(lb);
        box.check_name_or_throw("_p:m:world");
        box.content.check_marker(serialise::marker::poly_list);
        auto const items = box.content.extract_size_t();
        for (std::size_t index{}; index * 2 < items; ++index) {
            auto const pos = serialise::load_type<coordinates>(box.content);
            Chunk &chunk = w.chunk_at(pos);
            load(box.content, chunk);
        }
        box.check_empty_or_throw();
    }


}


namespace planet::map::hex {


    /// ## `planet::hexmap::world`


    template<typename Chunk>
    void save(serialise::save_buffer &ab, world<Chunk> const &w) {
        ab.save_box("_p:h:world", w.grid);
    }
    template<typename Chunk>
    void load(serialise::load_buffer &lb, world<Chunk> &w) {
        lb.load_box("_p:h:world", w.grid);
    }


}
