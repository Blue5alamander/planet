#include <planet/camera/orthogonal_birdseye.hpp>
#include <planet/camera/target3d.hpp>
#include <planet/log.hpp>


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
    auto const p2 = perspective.into() * view.into(p);
    return {p2.x(), p2.y()};
}


planet::affine::point3d planet::camera::target3dxy::out_of_for_z(
        affine::point2d const &xy, float const z) {
    static affine::point3d constexpr xynormal{0, 0, 1};

    /// Centre of the plane in world coordinates
    affine::point3d const plane_centre{0, 0, z};
    /// Start and end of the camera ray in camera coordinates
    float const fov_rad = current.perspective_fov * pi / 2.0f;
    float const focal_length =
            current.perspective_scale / std::tan(fov_rad / 2.0f);
    affine::point3d const end_camera{xy, -focal_length};
    /// Convert start & end to world coordinates
    auto const start_world = focal_point();
    auto const end_world = view.outof(end_camera);
    /// Direction of the ray
    auto const dir{(end_world - start_world).as_unit_vector()};
    /// Ray intersection algorithm
    auto const dot_normal = dir.dot(xynormal);
    if (dot_normal == 0.0f) {
        planet::log::warning(
                "Camera ray is parallel to xy plane. Position", xy,
                "start_world", start_world, "end_world", end_world, "direction",
                dir);
        return plane_centre;
    } else {
        auto const numerator = xynormal.dot(plane_centre - start_world);
        auto const t = numerator / dot_normal;
        if (t > 0.00001f) {
            return {(start_world + dir * t).xy(), z};
        } else {
            planet::log::warning(
                    "Camera ray points away from the plane. Position", xy,
                    "start_world", start_world, "end_world", end_world,
                    "direction", dir);
            return plane_centre;
        }
    }
}


void planet::camera::target3dxy::tick_update() {
    ortho.tick_update();
    /// TODO Do the animated current update from the target
    current = target;
    view = {ortho.view.into(), ortho.view.outof()};
    view.scale(1.0f, 1.0f, 1.0f / ortho.current_scale)
            .translate(current.view_offset)
            .rotate_x(current.view_angle)
            .translate(current.view_direction * current.distance);
    perspective = affine::transform3d::perspective(
            current.perspective_scale, current.perspective_fov);
}
felspar::coro::task<void>
        planet::camera::target3dxy::updates(felspar::io::warden &warden) {
    while (true) {
        tick_update();
        co_await warden.sleep(25ms);
    }
}
