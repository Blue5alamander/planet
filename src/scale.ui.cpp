#include <planet/ui/scale.hpp>

#include <iostream>


namespace {
    float scale_factor(
            float const distance,
            float const bound,
            bool const shrink,
            bool const expand) {
        std::cout << "d " << distance << " b " << bound
                  << (shrink ? " shrink" : "") << (expand ? " expand" : "");
        if (distance > bound and shrink or distance < bound and expand) {
            std::cout << " b/d " << (bound / distance) << '\n';
            return bound / distance;
        } else {
            std::cout << " 1.0f\n";
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
    return {size.width * xf, size.height * yf};
}
