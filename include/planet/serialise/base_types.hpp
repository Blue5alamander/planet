#pragma once


#include <planet/serialise/load_buffer.hpp>
#include <planet/serialise/save_buffer.hpp>

#include <array>
#include <vector>


namespace planet::serialise {


    inline void save(save_buffer &ab, bool const b) {
        ab.append(static_cast<std::uint8_t>(
                b ? marker::b_true : marker::b_false));
    }
    inline void load(load_buffer &lb, bool &b) {
        auto const m = lb.extract_marker();
        if (m == marker::b_true) {
            b = true;
        } else if (m == marker::b_false) {
            b = false;
        } else {
            throw wanted_boolean(lb.cmemory(), m);
        }
    }


    template<felspar::parse::concepts::integral T>
    inline void save(save_buffer &ab, T const t) {
        ab.append(static_cast<std::uint8_t>(marker_for<T>()));
        ab.append(t);
    }
    template<felspar::parse::concepts::integral T>
    inline void load(load_buffer &lb, T &s) {
        auto const m = lb.extract_marker();
        if (auto const want = marker_for<T>(); m != want) {
            throw wrong_marker(lb.cmemory(), want, m);
        } else {
            s = lb.extract<T>();
        }
    }


    template<typename T, std::size_t N>
    inline void save(save_buffer &ab, std::array<T, N> const &a) {
        ab.append_size_t(a.size());
        for (auto &&item : a) { save(ab, item); }
    }
    template<typename T, std::size_t N>
    void load(load_buffer &lb, std::array<T, N> &a) {
        auto const items = lb.extract_size_t();
        if (items > N) {
            throw felspar::stdexcept::runtime_error{"Too many items for array"};
        }
        for (auto &item : a) { load(lb, item); }
    }


    template<typename T>
    inline void save(save_buffer &ab, std::vector<T> const &v) {
        ab.append_size_t(v.size());
        for (auto &&item : v) { save(ab, item); }
    }
    template<typename T>
    inline void load(load_buffer &lb, std::vector<T> &v) {
        auto const items = lb.extract_size_t();
        v = std::vector<T>(items);
        for (auto &item : v) { load(lb, item); }
    }


}
