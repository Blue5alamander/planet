#pragma once


#include <planet/affine/transform2d.hpp>

#include <felspar/coro/task.hpp>
#include <felspar/io/warden.hpp>


namespace planet::camera {


    /// ## Bird's eye view of the XY plane
    /**
     * A camera that looks down on the XY plane without any perspective
     * projection. The location that the camera is looking at can be set using
     * `target_looking_at` and the zoom level can be set using `target_scale`.
     * If the `updates` coroutine is used then the camera will animate from the
     * current position and scale to the targets.
     */
    class orthogonal_birdseye {
      public:
        planet::affine::transform2d view;

        /// ### Updates the camera view over time to match the targets
        /**
         * Should be run in a task manager, e.g.:
         * ```cpp
         * felspar::coro::starter<> tasks;
         * tasks.post(
         *      camera,
         *      &planet::camera::orthogonal_birdseye::updates,
         *      std::ref(warden));
         * ```
         */
        felspar::coro::task<void> updates(felspar::io::warden &warden);

        /// ### Targets

        /// #### Targets used by `updates`
        planet::affine::point2d target_looking_at{};
        float target_scale{1};
        float target_rotation{};
        /// #### Values used for the projections matrix
        planet::affine::point2d current_looking_at = target_looking_at;
        float current_scale = target_scale;
        float current_rotation = target_rotation;
    };


}
