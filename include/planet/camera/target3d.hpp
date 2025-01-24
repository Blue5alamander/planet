#pragma once


#include <planet/affine/transform3d.hpp>
#include <planet/camera/orthogonal_birdseye.hpp>


namespace planet::camera {


    /// ## 3D Look-at-target camera
    /**
     * The camera targets a location in 3d space, but aligns itself as if the
     * world is built in the XY plane with the Z axis pointing up.
     *
     * The `orthogonal_birdseye` camera provides an initial set of transforms that provide basic rotation, position and scaling support. This is then further adjusted by the 3D transforms held by this camera.
     */
    struct target3dxy {
        /// ### Initial transformation
        orthogonal_birdseye initial;


        /// ### Set up parameters
        struct parameters {
            /// #### Target parameters
            affine::point3d looking_at{};
            /// ##### The distance away from the target
            float distance{3};
            /// ##### Rotation around the Z axis
            float rotation{};
            /// ##### View angle
            /**
             * This is how oblique the camera is to looking at the ground. An
             * angle of zero means that the camera is pointing straight down,
             * and an angle of one means it is pointing straight up.
             */
            float view_angle{0.25f};
        };

        /// ### Targetting
        parameters target = {}, currently = target;

        /// #### Camera position and direction
        affine::point3d current_view_direction{0, 0, -1};
        affine::point3d current_focal_point =
                current_view_direction * -currently.distance;


        /// ### Return the transform for the current parameters
        affine::transform3d current_transform() const;


        /// ### Current matrix transforms
        affine::transform3d view = current_transform();

        /// #### The `into` transform
        affine::point2d into(affine::point3d const &);

        /// #### The `outof` transform
        /**
         * This takes the 2d point in the screen space and returns the point on
         * the XY plane at height Z that is being looked at.
         */
        affine::point3d out_of_for_z(affine::point2d const &, float z);


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
    };


}
