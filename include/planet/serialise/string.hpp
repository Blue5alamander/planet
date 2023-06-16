#pragma once


#include <planet/serialise/forward.hpp>

#include <string>


namespace planet::serialise {


    void save(save_buffer &, std::string_view);
    template<std::size_t N>
    void save(save_buffer &ab, const char (&s)[N]) {
        save(ab, std::string_view(s));
    }
    inline void save(save_buffer &ab, char const *s) {
        save(ab, std::string_view(s));
    }
    void load(load_buffer &, std::string &);


}
