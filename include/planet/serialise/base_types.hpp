#pragma once


#include <planet/serialise/save_buffer.hpp>


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
    inline save_buffer &save(save_buffer &ab, std::string_view sv) {
        ab.append(sv);
        return ab;
    }
    inline void load(std::span<std::byte const> &l, std::string_view &sv) {
        auto bytes = load_type<std::size_t>(l);
        sv = {reinterpret_cast<char const *>(l.data()), bytes};
        l = l.subspan(bytes);
    }


}
