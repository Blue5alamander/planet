#pragma once


#include <planet/serialise/base_types.hpp>
#include <planet/serialise/save_buffer.hpp>
#include <planet/serialise/load.hpp>

#include <planet/map/hex.hpp>

#include <vector>


namespace planet::serialise {


    // Find a better home for all of these


    template<typename T, std::size_t N>
    inline save_buffer &save(save_buffer &ab, std::span<T, N> const &a) {
        ab.append(a.size());
        for (auto const &item : a) { save(ab, item); }
        return ab;
    }
    template<typename T>
    inline save_buffer &save(save_buffer &ab, std::vector<T> const &v) {
        ab.append(v.size());
        for (auto const &item : v) { save(ab, item); }
        return ab;
    }

    template<typename T>
    inline save_buffer &save(save_buffer &ab, felspar::coro::generator<T> gen) {
        for (auto &&item : gen) { return save(ab, item); }
        return ab;
    }

    inline save_buffer &save(save_buffer &ab, planet::map::coordinates const c) {
        return ab.save_box("mapcordinates", c.column(), c.row());
    }

    template<typename Cell, std::size_t DimX, std::size_t DimY = DimX>
    inline save_buffer &
            save(save_buffer &ab,
                 planet::map::chunk<Cell, DimX, DimY> *const c) {
        return ab.save_box("mapchunk", c->cells());
    }

    template<typename Co, typename Chunk>
    inline save_buffer &save(save_buffer &ab, std::pair<Co, Chunk *> const c) {
        return ab.save_box("mapchunkpair", c.first, c.second);
    }

    template<typename Chunk>
    inline save_buffer &save(save_buffer &ab, planet::map::world<Chunk> &w) {
        return ab.save_box("world", w.chunks());
    }

    template<typename Chunk>
    inline save_buffer &save(save_buffer &ab, planet::hexmap::world<Chunk> &w) {
        return ab.save_box("hexworld", w.chunks());
    }


    inline save_buffer &
            save(save_buffer &ab, planet::hexmap::coordinates const c) {
        return ab.save_box("hexc", c.compressed());
    }


}
