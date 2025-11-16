#pragma once


#include <planet/affine/point3d.hpp>
#include <planet/serialise/forward.hpp>
#include <planet/vertex/forward.hpp>
#include <planet/vertex/uvpos.hpp>


namespace planet::vertex {


    struct normal_textured {
        constexpr static std::string_view box{"_p:vert:ntx"};


        affine::point3d p, n;
        uvpos uv;
    };
    void save(serialise::save_buffer &, normal_textured const &);
    void load(serialise::box &, normal_textured &);


}
