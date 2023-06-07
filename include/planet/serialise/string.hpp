#pragma once


#include <planet/serialise/forward.hpp>

#include <string>


namespace planet::serialise {


    void save(save_buffer &, std::string const &);
    void load(load_buffer &, std::string &);


}
