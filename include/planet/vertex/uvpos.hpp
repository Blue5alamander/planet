#pragma once


#include <planet/serialise/forward.hpp>


namespace planet::vertex {


    /// ## UV texture position
    struct uvpos {
        constexpr static std::string_view box{"_p:vert:uv"};


        float u = {}, v = {};
        friend constexpr uvpos operator+(uvpos const l, uvpos const r) {
            return {l.u + r.u, l.v + r.v};
        }
    };
    void save(serialise::save_buffer &, uvpos const &);
    void load(serialise::box &, uvpos &);


}
