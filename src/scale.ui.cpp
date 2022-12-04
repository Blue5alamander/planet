#include <planet/ui/scale.hpp>

#include <algorithm>


namespace {
    float scale_factor(
            float const distance,
            float const bound,
            bool const shrink,
            bool const expand) {
        if (distance > bound and shrink or distance < bound and expand) {
            return bound / distance;
        } else {
            return 1.0f;
        }
    }
}


planet::affine::extents2d planet::ui::scaling(
        affine::extents2d const size,
        affine::extents2d const bounds,
        scale const fit) {
    auto const xf = scale_factor(
            size.width, bounds.width, fit bitand scale::shrink_x,
            fit bitand scale::expand_x);
    auto const yf = scale_factor(
            size.height, bounds.height, fit bitand scale::shrink_y,
            fit bitand scale::expand_y);
    if (xf != yf and fit bitand 128) {
        auto const sf = std::min(xf, yf);
        return {size.width * sf, size.height * sf};
    } else {
        return {size.width * xf, size.height * yf};
    }
}
