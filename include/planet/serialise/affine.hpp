#pragma once


#include <planet/affine/forward.hpp>
#include <planet/serialise/forward.hpp>


namespace planet::affine {


    /// ## Save and load various affine types

    void save(serialise::save_buffer &, point2d const &);
    void load(serialise::box &, point2d &);


}
