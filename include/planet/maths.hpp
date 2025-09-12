#pragma once


#include <planet/numbers.hpp>

#include <cmath>


namespace planet {


    inline auto sin(float const turn) {
        return std::sin(std::fmod(turn, 1.0f) * tau);
    }
    inline auto cos(float const turn) {
        return std::cos(std::fmod(turn, 1.0f) * tau);
    }


}
