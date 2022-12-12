#include <planet/drawing/image.hpp>


planet::drawing::image::image(
        std::size_t const w, std::size_t const h, rgba8bpc const c)
: pixels(w * h, c), width{w} {}


std::span<planet::drawing::rgba8bpc>
        planet::drawing::image::operator[](std::size_t row) {
    return {pixels.data() + width * row, width};
}
std::span<planet::drawing::rgba8bpc const>
        planet::drawing::image::operator[](std::size_t row) const {
    return {pixels.data() + width * row, width};
}
