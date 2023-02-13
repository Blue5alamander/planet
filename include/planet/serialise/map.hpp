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


    /// ### `planet::map::world`


    template<typename Chunk>
    inline serialise::save_buffer &
            save(serialise::save_buffer &ab, world<Chunk> const &w) {
        return ab.save_box("_p:m:world", w.storage);
    }


}


namespace planet::hexmap {


    /// ### `planet::hexmap::world`


    template<typename Chunk>
    inline serialise::save_buffer &
            save(serialise::save_buffer &ab, world<Chunk> const &w) {
        return ab.save_box("_p:h:world", w.grid);
    }


}
