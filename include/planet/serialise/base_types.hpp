#pragma once


#include <planet/serialise/load.hpp>
#include <planet/serialise/save_buffer.hpp>

#include <array>
#include <vector>


namespace planet::serialise {


    template<felspar::parse::concepts::integral T>
    inline save_buffer &save(save_buffer &ab, T const t) {
        ab.append(t);
        return ab;
    }
    template<felspar::parse::concepts::integral T>
    inline void load(load_buffer &l, T &s) {
        s = l.extract<T>();
    }


    inline save_buffer &save(save_buffer &ab, std::string_view const sv) {
        ab.append(sv);
        return ab;
    }
    inline void load(load_buffer &l, std::string_view &sv) {
        auto bytes = load_type<std::size_t>(l);
        sv = {reinterpret_cast<char const *>(l.split(bytes).data()), bytes};
    }


    template<typename P>
    inline save_buffer &save(save_buffer &ab, std::unique_ptr<P> const &p) {
        if (p) {
            return ab.save_box("_s:uqp:v", *p);
        } else {
            return ab.save_box("_s:uqp:e");
        }
    }
    template<typename P>
    inline void load(load_buffer &l, std::unique_ptr<P> &p) {
        auto b = load_type<box>(l);
        if (b.name == "_s:uqp:v") {
            p = std::make_unique<P>(load_type<P>(b.content));
        } else if (b.name == "_s:uqp:e") {
            p.reset();
        } else {
            throw felspar::stdexcept::runtime_error{"Unexpected box name"};
        }
    }


    template<typename F, typename S>
    inline save_buffer &save(save_buffer &ab, std::pair<F, S> const &p) {
        save(ab, p.first);
        save(ab, p.second);
        return ab;
    }
    template<typename F, typename S>
    inline void load(load_buffer &l, std::pair<F, S> &p) {
        load(l, p.first);
        load(l, p.second);
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
