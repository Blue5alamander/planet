#include <planet/affine2d.hpp>
#include <planet/serialise/affine.hpp>
#include <planet/serialise/base_types.hpp>
#include <planet/serialise/save_buffer.hpp>


void planet::affine::save(serialise::save_buffer &ab, point2d const &p) {
    ab.save_box("_p:_a:point2d", p.x(), p.y());
}
void planet::affine::load(serialise::box &box, point2d &p) {
    float x, y;
    box.named("_p:_a:point2d", x, y);
    p = point2d{x, y};
}
