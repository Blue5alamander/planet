#pragma once


#include <planet/serialise/load_buffer.hpp>
#include <planet/serialise/save_buffer.hpp>

#include <chrono>


namespace planet::serialise {


    inline void
            save(save_buffer &ab,
                 std::chrono::system_clock::time_point const &tp) {
        ab.save_box("_sc::system_clock::tp", tp.time_since_epoch().count());
    }
    inline void
            load(load_buffer &lb, std::chrono::system_clock::time_point &tp) {
        decltype(tp.time_since_epoch().count()) c;
        lb.load_box("_sc::system_clock::tp", c);
        tp = std::chrono::system_clock::time_point{
                std::chrono::system_clock::duration{c}};
    }


}
