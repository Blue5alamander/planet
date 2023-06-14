#include <planet/affine2d.hpp>
#include <planet/serialise/affine.hpp>
#include <planet/serialise/base_types.hpp>
#include <planet/serialise/save_buffer.hpp>


void planet::affine::save(serialise::save_buffer &ab, matrix2d const &m) {
    ab.save_box("_p:_a:matrix2d", m.cmemory());
}
void planet::affine::load(serialise::box &box, matrix2d &m) {
    box.named("_p:_a:matrix2d", m.memory());
}


void planet::affine::save(serialise::save_buffer &ab, point2d const &p) {
    ab.save_box("_p:_a:point2d", p.x(), p.y());
}
void planet::affine::load(serialise::box &box, point2d &p) {
    float x, y;
    box.named("_p:_a:point2d", x, y);
    p = point2d{x, y};
}


void planet::affine::save(serialise::save_buffer &ab, transform2d const &t) {
    ab.save_box("_p:_a:transform2d", t.in, t.out);
}
void planet::affine::load(serialise::box &box, transform2d &t) {
    box.named("_p:_a:transform2d", t.in, t.out);
}
