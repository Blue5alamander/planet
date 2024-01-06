#pragma once


#include <planet/serialise/forward.hpp>

#include <filesystem>
#include <string>


namespace planet::serialise {


    void save(save_buffer &, std::string_view);
    inline void save(save_buffer &ab, std::string const &s) {
        save(ab, std::string_view{s});
    }
    template<std::size_t N>
    void save(save_buffer &ab, const char (&s)[N]) {
        save(ab, std::string_view(s));
    }
    inline void save(save_buffer &ab, char const *s) {
        save(ab, std::string_view(s));
    }
    void load(load_buffer &, std::string &);
    void load(load_buffer &, std::string_view &);


    inline void save(save_buffer &ab, std::filesystem::path const &p) {
        save(ab, p.string());
    }
    inline void load(load_buffer &ab, std::filesystem::path &p) {
        std::string t;
        load(ab, t);
        p = t;
    }


}
