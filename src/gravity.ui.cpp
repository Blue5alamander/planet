#include <planet/ui/gravity.hpp>

#include <utility>


namespace {
    std::pair<float, float>
            spacing(float const low,
                    float const high,
                    float const size,
                    bool const at_low,
                    bool const at_high) noexcept {
        if (at_low) {
            return {low, low + size};
        } else if (at_high) {
            return {high - size, high};
        } else {
            auto const padding = (high - low - size) / 2;
            return {low + padding, high - padding};
        }
    }
}


planet::affine::rectangle2d planet::ui::within(
        gravity const g,
        affine::rectangle2d const &o,
        affine::extents2d const &i) noexcept {
    auto const [left, right] = spacing(
            o.top_left.x(), o.top_left.x() + o.extents.width, i.width,
            g.horizontally == gravity::left, g.horizontally == gravity::right);
    auto const [top, bottom] = spacing(
            o.top_left.y(), o.top_left.y() + o.extents.height, i.height,
            g.vertically == gravity::top, g.vertically == gravity::bottom);
    return {{left, top}, affine::point2d{right, bottom}};
}
