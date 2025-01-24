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
        planet::camera::target3dxy::current_transform() const {
    float const aspect = 1.0f;
    float const theta = 1.0f;
    float const n = -1.0f;
    float const f = -100.0f;

    return affine::transform3d::perspective(aspect, theta, n, f);
}


felspar::coro::task<void>
        planet::camera::target3dxy::updates(felspar::io::warden &warden) {

    while (true) {
        initial.tick_update();
        view = {initial.view.into(), initial.view.outof()};
        co_await warden.sleep(25ms);
    }
}
