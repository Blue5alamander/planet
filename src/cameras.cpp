#include <planet/camera/orthogonal_birdseye.hpp>
#include <planet/camera/target3d.hpp>


using namespace std::literals;


/// ## `planet::camera::orthogonal_birdseye`


void planet::camera::orthogonal_birdseye::tick_update() {
    auto const scale_difference = target_scale - current_scale;
    current_scale += scale_difference * 0.15f;

    auto const rotation_difference = target_rotation - current_rotation;
    current_rotation += rotation_difference * 0.1f;

    auto const direction = target_looking_at - current_looking_at;
    if (direction.mag2() <= 0.0001f) {
        current_looking_at = target_looking_at;
    } else {
        auto const translate = planet::affine::point2d::from_polar(
                std::sqrt(direction.mag2()) * 0.15f, direction.theta());
        current_looking_at = current_looking_at + translate;
    }

    view = {};
    view.translate(-current_looking_at)
            .rotate(current_rotation)
            .scale(1.0f / current_scale);
}
felspar::coro::task<void> planet::camera::orthogonal_birdseye::updates(
        felspar::io::warden &warden) {
    while (true) {
        tick_update();
        co_await warden.sleep(25ms);
    }
}


/// ## `planet::camera::target3dxy`


planet::camera::target3dxy::target3dxy(parameters const &p) : target{p} {
    tick_update();
}


planet::affine::point2d
        planet::camera::target3dxy::into(affine::point3d const &p) {
    auto const p2 = view.into(p);
    return {p2.xh, p2.yh, p2.h};
}


planet::affine::point3d planet::camera::target3dxy::out_of_for_z(
        affine::point2d const &, float) {
    return {};
}


planet::affine::transform3d
        planet::camera::target3dxy::current_perspective_transform() const {
    float const scale = 3.333f;
    float const theta = 1.0f;
    return affine::transform3d::perspective(scale, theta);
}


void planet::camera::target3dxy::tick_update() {
    initial.tick_update();
    /// TODO Do the current update from the target
    view = {initial.view.into(), initial.view.outof()};
    view.scale(1.0f, 1.0f, 1.0f / initial.current_scale)
            .translate(current.view_offset)
            .rotate_x(-0.125f)
            .translate(current.view_direction * current.distance);
}
felspar::coro::task<void>
        planet::camera::target3dxy::updates(felspar::io::warden &warden) {
    while (true) {
        tick_update();
        co_await warden.sleep(25ms);
    }
}
