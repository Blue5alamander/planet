#include <planet/affine2d.hpp>
#include <planet/affine3d.hpp>
#include <planet/log.hpp>
#include <planet/ostream.hpp>
#include <planet/serialise/affine.hpp>
#include <planet/serialise/base_types.hpp>
#include <planet/serialise/save_buffer.hpp>


/// ## `planet::affine::extents2d`


void planet::affine::save(serialise::save_buffer &ab, extents2d const &t) {
    ab.save_box("_p:a:e2", t.width, t.height);
}
void planet::affine::load(serialise::box &box, extents2d &t) {
    box.named("_p:a:e2", t.width, t.height);
}
namespace {
    auto const e2 = planet::log::format("_p:a:e2", [](auto &os, auto &box) {
        os << planet::serialise::load_from_box<planet::affine::extents2d>(box);
    });
}


/// ## `planet::affine::matrix2d`


void planet::affine::save(serialise::save_buffer &ab, matrix2d const &m) {
    ab.save_box("_p:a:m2", m.cmemory());
}
void planet::affine::load(serialise::box &box, matrix2d &m) {
    box.named("_p:a:m2", m.memory());
}


/// ## `planet::affine::point2d`


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
        os << planet::serialise::load_from_box<planet::affine::point2d>(box);
    });
}


/// ## `planet::affine::rectangle2d`


void planet::affine::save(serialise::save_buffer &ab, rectangle2d const &t) {
    ab.save_box("_p:a:r2", t.top_left, t.extents);
}
void planet::affine::load(serialise::box &box, rectangle2d &t) {
    box.named("_p:a:r2", t.top_left, t.extents);
}
namespace {
    auto const r2 = planet::log::format("_p:a:r2", [](auto &os, auto &box) {
        os << planet::serialise::load_from_box<planet::affine::rectangle2d>(
                box);
    });
}


/// ## `planet::affine::transform2d`


void planet::affine::save(serialise::save_buffer &ab, transform2d const &t) {
    ab.save_box("_p:a:t2", t.in, t.out);
}
void planet::affine::load(serialise::box &box, transform2d &t) {
    box.named("_p:a:t2", t.in, t.out);
}


/// ## `planet::affine::transform3d`


planet::affine::transform3d &
        planet::affine::transform3d::translate(point3d const &p) {
    in = matrix3d::translate(p.x(), p.y(), p.z()) * in;
    out = out * matrix3d::translate(-p.x(), -p.y(), -p.z());
    return *this;
}

planet::affine::transform3d &
        planet::affine::transform3d::rotate_x(float const t) {
    in = matrix3d::rotate_x(t) * in;
    out = out * matrix3d::rotate_x(-t);
    return *this;
}


planet::affine::transform3d planet::affine::transform3d::perspective(
        float const aspect, float const theta, float const n, float const f) {
    float const fov_rad = theta * pi;
    float const focal_length = 1.0f / std::tan(fov_rad / 2.0f);
    float const x = focal_length / aspect;
    float const y = -focal_length;
    float const A = n / (f - n);
    float const B = f * A;

    return {{std::array{
                    std::array{x, 0.0f, 0.0f, 0.0f},
                    std::array{0.0f, y, 0.0f, 0.0f},
                    std::array{0.0f, 0.0f, A, B},
                    std::array{0.0f, 0.0f, -1.0f, 0.0f}}},

            {std::array{
                    std::array{1.0f / x, 0.0f, 0.0f, 0.0f},
                    std::array{0.0f, 1.0f / y, 0.0f, 0.0f},
                    std::array{0.0f, 0.0f, 0.0f, -1.0f},
                    std::array{0.0f, 0.0f, 1.0f / A, A / B}}}};
}


planet::affine::transform3d
        planet::affine::operator*(transform3d const &l, transform3d const &r) {
    return {l.into() * r.into(), r.outof() * l.outof()};
}
