#pragma once


#include <planet/affine/forward.hpp>
#include <planet/serialise/forward.hpp>


namespace planet::affine {


    /// ## Save and load various affine types

    void save(serialise::save_buffer &, extents2d const &);
    void load(serialise::box &, extents2d &);

    void save(serialise::save_buffer &, matrix2d const &);
    void load(serialise::box &, matrix2d &);

    void save(serialise::save_buffer &, point2d const &);
    void load(serialise::box &, point2d &);

    void save(serialise::save_buffer &, point3d const &);
    void load(serialise::box &, point3d &);

    void save(serialise::save_buffer &, rectangle2d const &);
    void load(serialise::box &, rectangle2d &);


}
