#include <planet/affine2d.hpp>
#include <planet/log.hpp>
#include <planet/ostream.hpp>
#include <planet/serialise/affine.hpp>
#include <planet/serialise/base_types.hpp>
#include <planet/serialise/save_buffer.hpp>


void planet::affine::save(serialise::save_buffer &ab, extents2d const &t) {
    ab.save_box("_p:a:e2", t.width, t.height);
}
void planet::affine::load(serialise::box &box, extents2d &t) {
    box.named("_p:a:e2", t.width, t.height);
}
namespace {
    auto const e2 = planet::log::format("_p:a:e2", [](auto &os, auto &box) {
        os << planet::serialise::load_type<planet::affine::extents2d>(box);
    });
}


void planet::affine::save(serialise::save_buffer &ab, matrix2d const &m) {
    ab.save_box("_p:a:m2", m.cmemory());
}
void planet::affine::load(serialise::box &box, matrix2d &m) {
    box.named("_p:a:m2", m.memory());
}


void planet::affine::save(serialise::save_buffer &ab, point2d const &p) {
    ab.save_box("_p:a:p2", p.x(), p.y());
}
void planet::affine::load(serialise::box &box, point2d &p) {
    float x, y;
    box.named("_p:a:p2", x, y);
    p = point2d{x, y};
}
namespace {
    auto const p2 = planet::log::format("_p:a:p2", [](auto &os, auto &box) {
        os << planet::serialise::load_type<planet::affine::point2d>(box);
    });
}


void planet::affine::save(serialise::save_buffer &ab, rectangle2d const &t) {
    ab.save_box("_p:a:r2", t.top_left, t.extents);
}
void planet::affine::load(serialise::box &box, rectangle2d &t) {
    box.named("_p:a:r2", t.top_left, t.extents);
}
namespace {
    auto const r2 = planet::log::format("_p:a:r2", [](auto &os, auto &box) {
        os << planet::serialise::load_type<planet::affine::rectangle2d>(box);
    });
}


void planet::affine::save(serialise::save_buffer &ab, transform2d const &t) {
    ab.save_box("_p:a:t2", t.in, t.out);
}
void planet::affine::load(serialise::box &box, transform2d &t) {
    box.named("_p:a:t2", t.in, t.out);
}
