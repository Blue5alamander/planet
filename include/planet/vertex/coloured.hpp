#pragma once


#include <planet/affine/point3d.hpp>
#include <planet/serialise/forward.hpp>
#include <planet/colour.hpp>
#include <planet/vertex/forward.hpp>


namespace planet::vertex {


    struct coloured {
        constexpr static std::string_view box{"_p:vert:col"};


        affine::point3d p;
        colour col = colour::white;
    };
    void save(serialise::save_buffer &, coloured const &);
    void load(serialise::box &, coloured &);


}
