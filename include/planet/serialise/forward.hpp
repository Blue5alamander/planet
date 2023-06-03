#pragma once


#include <felspar/parse/concepts.hpp>

#include <span>
#include <string_view>


namespace planet::serialise {


    struct box;
    class load_buffer;
    class save_buffer;


    template<typename S>
    S load_type(load_buffer &);


}
