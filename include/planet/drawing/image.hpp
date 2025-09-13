#pragma once


#include <planet/drawing/colour.hpp>

#include <span>
#include <vector>


namespace planet::drawing {


    /// ## Image using a 32 bit RGBA format
    class image {
        std::vector<rgba8bpc> pixels;
        std::size_t width;

      public:
        image(std::size_t width, std::size_t height, rgba8bpc);

        std::span<rgba8bpc> operator[](std::size_t row);
        std::span<rgba8bpc const> operator[](std::size_t row) const;
    };


}
