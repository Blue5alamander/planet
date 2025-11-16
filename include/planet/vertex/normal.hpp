#pragma once


#include <planet/affine/point3d.hpp>
#include <planet/serialise/forward.hpp>
#include <planet/vertex/forward.hpp>


namespace planet::vertex {


    struct normal {
        constexpr static std::string_view box{"_p:vert:n"};


        affine::point3d p, n;
    };
    void save(serialise::save_buffer &, normal const &);
    void load(serialise::box &, normal &);


}
