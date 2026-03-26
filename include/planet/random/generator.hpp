#pragma once


#include <random>


namespace planet::random {


    /// ## Random generator
    inline thread_local std::random_device device;
    inline thread_local std::mt19937 generator(device());


}
