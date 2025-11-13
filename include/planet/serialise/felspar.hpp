#pragma once


#include <planet/serialise/base_types.hpp>
#include <planet/serialise/load_buffer.hpp>
#include <planet/serialise/save_buffer.hpp>

#include <felspar/memory/shared_buffer.hpp>
#include <felspar/memory/small_vector.hpp>


namespace planet::serialise {


    template<typename T, std::size_t C>
    void save(save_buffer &ab, felspar::memory::small_vector<T, C> const &v) {
        save(ab, std::span{v});
    }
    template<typename T, std::size_t N>
    void load(load_buffer &lb, felspar::memory::small_vector<T, N> &a) {
        lb.check_marker(marker::poly_list);
        auto const items = lb.extract_size_t();
        if (items > N) {
            throw felspar::stdexcept::runtime_error{
                    "Too many items for small_vector"};
        }
        a.resize(items);
        for (auto &item : a) { load(lb, item); }
    }


    template<typename T>
    void save(save_buffer &sb, felspar::memory::shared_buffer<T> const &b) {
        save(sb, std::span<T const>{b});
    }


    void save(save_buffer &, std::source_location const &);


}
