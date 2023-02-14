#pragma once


#include <planet/serialise/forward.hpp>

#include <felspar/memory/shared_vector.hpp>
#include <felspar/parse/extract.hpp>


namespace planet::serialise {


    struct box {
        std::string_view name;
        std::span<std::byte const> content;

        void check_name_or_throw(std::string_view expected) const {
            if (name != expected) {
                throw felspar::stdexcept::runtime_error{"Unexpected box name"};
            }
        }
        void check_empty_or_throw() const {
            if (not content.empty()) {
                throw felspar::stdexcept::runtime_error{
                        "Box was not empty after loading"};
            }
        }

        friend void load(std::span<std::byte const> &l, box &b) {
            b.name = load_type<std::string_view>(l);
            [[maybe_unused]] auto const version = load_type<std::uint8_t>(l);
            auto const bytes = load_type<std::size_t>(l);
            b.content = {l.data(), bytes};
            l = l.subspan(bytes);
        }
    };


    template<typename... Args>
    inline void load_box(
            std::span<std::byte const> &l,
            std::string_view name,
            Args &...args) {
        auto b = load_type<box>(l);
        b.check_name_or_throw(name);
        (load(b.content, args), ...);
        b.check_empty_or_throw();
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
