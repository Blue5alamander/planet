#pragma once


#include <planet/serialise/forward.hpp>

#include <felspar/memory/shared_vector.hpp>
#include <felspar/parse/extract.hpp>


namespace planet::serialise {


    template<typename... Args>
    inline void load_box(
            std::span<std::byte const> &l,
            std::string_view name,
            Args &...args) {
        auto const n = load_type<std::string_view>(l);
        if (n != name) {
            throw felspar::stdexcept::runtime_error{"Unexpected box name"};
        } else {
            [[maybe_unused]] auto const version = load_type<std::uint8_t>(l);
            auto const bytes = load_type<std::size_t>(l);
            std::span<std::byte const> content{l.data(), bytes};
            (load(content, args), ...);
            l = l.subspan(bytes);
        }
    }


    template<typename S>
    inline S load_type(std::span<std::byte const> &v) {
        S s;
        load(v, s);
        return s;
    }
    template<typename S>
    inline S load_type(felspar::memory::shared_byte_view v) {
        auto b = v.cmemory();
        auto s{load_type<S>(b)};
        if (not b.empty()) {
            throw felspar::stdexcept::runtime_error{
                    "There is still data in the buffer after loading the type"};
        }
        return s;
    }


}
