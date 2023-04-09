#include <planet/camera/orthogonal_birdseye.hpp>


using namespace std::literals;


/// ## `planet::camera::orthogonal_birdseye`


felspar::coro::task<void> planet::camera::orthogonal_birdseye::updates(
        felspar::io::warden &warden) {
    while (true) {
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

        co_await warden.sleep(25ms);
    }
}
