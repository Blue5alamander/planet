#pragma once


#include <planet/affine/point3d.hpp>
#include <planet/colour.hpp>
#include <planet/vertex/forward.hpp>
#include <planet/vertex/uvpos.hpp>


namespace planet::vertex {


    /// ## Basic textured mesh vertex with colour
    struct coloured_textured {
        constexpr static std::string_view box{"_p:vert:ctx"};


        planet::affine::point3d p;
        colour col = colour::white;
        uvpos uv;
    };
    void save(serialise::save_buffer &, coloured_textured const &);
    void load(serialise::box &, coloured_textured &);


}
