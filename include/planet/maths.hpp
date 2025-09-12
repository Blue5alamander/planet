#pragma once


#include <planet/numbers.hpp>

#include <cmath>


namespace planet {


    inline auto sin(float const turn) { return std::sin(turn * tau); }
    inline auto cos(float const turn) { return std::cos(turn * tau); }


}
