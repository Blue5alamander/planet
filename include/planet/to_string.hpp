#pragma once


#include <string>
#include <utility>


namespace planet {


    template<typename F, typename S>
    std::string to_string(std::pair<F, S> const &p) {
        return "(" + std::to_string(p.first) + ", " + std::to_string(p.second)
                + ")";
    }


}
