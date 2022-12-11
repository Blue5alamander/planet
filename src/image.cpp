#include <planet/drawing/image.hpp>


planet::drawing::image::image(
        std::size_t const width, std::size_t const height, rgba8bpc const c)
: pixels(width * height, c) {}
