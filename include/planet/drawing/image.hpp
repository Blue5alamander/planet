#pragma once


#include <planet/drawing/colour.hpp>

#include <vector>


namespace planet::drawing {


    /// Image using a 32 bit RGBA format
    class image {
        std::vector<rgba8bpc> pixels;

      public:
        image(std::size_t width, std::size_t height, rgba8bpc);
    };


}
