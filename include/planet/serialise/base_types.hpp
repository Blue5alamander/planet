#pragma once


#include <planet/serialise/save_buffer.hpp>

#include <array>
#include <vector>


namespace planet::serialise {


    template<typename S>
    S load_type(std::span<std::byte const> &);


    template<felspar::parse::detail::integral T>
    inline save_buffer &save(save_buffer &ab, T const t) {
        ab.append(t);
        return ab;
    }
    template<felspar::parse::detail::integral T>
    inline void load(std::span<std::byte const> &l, T &s) {
        s = felspar::parse::binary::extract<T>(l);
    }


    inline save_buffer &save(save_buffer &ab, std::string_view const sv) {
        ab.append(sv);
        return ab;
    }
    inline void load(std::span<std::byte const> &l, std::string_view &sv) {
        auto bytes = load_type<std::size_t>(l);
        sv = {reinterpret_cast<char const *>(l.data()), bytes};
        l = l.subspan(bytes);
    }


    template<typename P>
    inline save_buffer &save(save_buffer &ab, std::unique_ptr<P> const &p) {
        if (p) {
            return ab.save_box("_s:uqp:v", *p);
        } else {
            return ab.save_box("_s:uqp:e");
        }
    }


    template<typename F, typename S>
    inline save_buffer &save(save_buffer &ab, std::pair<F, S> const &p) {
        save(ab, p.first);
        save(ab, p.second);
        return ab;
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


}
