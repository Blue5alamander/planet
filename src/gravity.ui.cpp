#include <planet/ui/box.hpp>


namespace {
    std::pair<float, float>
            spacing(float left,
                    float right,
                    float const width,
                    bool const at_left,
                    bool const at_right) {
        if (at_left) {
            if (at_right) {
                auto const padding = (right - left - width) / 2;
                left += padding;
                right -= padding;
            } else {
                right = left + width;
            }
        } else if (at_right) {
            left = right - width;
        }
        return {left, right};
    }
}


planet::affine::rectangle planet::ui::within(
        gravity const g,
        affine::rectangle const &o,
        affine::extents2d const &i) {
    auto [left, right] =
            spacing(o.top_left.x(), o.top_left.x() + o.extents.width, i.width,
                    g bitand gravity::left, g bitand gravity::right);
    auto [top, bottom] =
            spacing(o.top_left.y(), o.top_left.y() + o.extents.height, i.height,
                    g bitand gravity::top, g bitand gravity::bottom);
    return {{left, top}, affine::point2d{right, bottom}};
}
