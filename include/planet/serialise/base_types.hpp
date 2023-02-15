#pragma once


#include <planet/serialise/load.hpp>
#include <planet/serialise/save_buffer.hpp>

#include <array>
#include <vector>


namespace planet::serialise {


    inline save_buffer &save(save_buffer &ab, bool const b) {
        ab.append(static_cast<std::uint8_t>(
                b ? marker::b_true : marker::b_false));
        return ab;
    }
    inline void load(load_buffer &l, bool &b) {
        auto const m = l.extract_marker();
        if (m == marker::b_true) {
            b = true;
        } else if (m == marker::b_false) {
            b = false;
        } else {
            throw wanted_boolean(m);
        }
    }


    template<felspar::parse::concepts::integral T>
    inline save_buffer &save(save_buffer &ab, T const t) {
        ab.append(static_cast<std::uint8_t>(marker_for<T>()));
        ab.append(t);
        return ab;
    }
    template<felspar::parse::concepts::integral T>
    inline void load(load_buffer &l, T &s) {
        auto const m = l.extract_marker();
        if (auto const want = marker_for<T>(); m != want) {
            throw wrong_marker(want, m);
        } else {
            s = l.extract<T>();
        }
    }


    template<typename T, std::size_t N>
    inline save_buffer &save(save_buffer &ab, std::span<T, N> const a) {
        ab.append(a.size());
        for (auto &&item : a) { save(ab, item); }
        return ab;
    }

    template<typename T, std::size_t N>
    inline save_buffer &save(save_buffer &ab, std::array<T, N> const &a) {
        return save(ab, std::span<T const, N>{a.data(), a.size()});
    }

    template<typename T>
    inline save_buffer &save(save_buffer &ab, std::vector<T> const &v) {
        return save(ab, std::span<T const>{v.data(), v.size()});
    }
    template<typename T>
    inline void load(load_buffer &l, std::vector<T> &v) {
        auto const items = load_type<std::size_t>(l);
        v = std::vector<T>(items);
        for (auto &item : v) { load(l, item); }
    }


}
