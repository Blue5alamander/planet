#pragma once


#include <felspar/parse/concepts.hpp>

#include <span>
#include <string_view>


namespace planet::serialise {


    class load_buffer;


    template<felspar::parse::concepts::integral T>
    void load(load_buffer &, T &);
    void load(load_buffer &, std::string_view &);


    template<typename S>
    S load_type(load_buffer &);


    class save_buffer;


}
