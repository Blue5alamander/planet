#pragma once


#include <planet/affine/transform3d.hpp>
#include <planet/camera/orthogonal_birdseye.hpp>


namespace planet::camera {


    /// ## 3D Look-at-target camera
    /**
     * The camera targets a location in 3d space, but aligns itself as if the
     * world is built in the XY plane with the Z axis pointing up.
     *
     * The `orthogonal_birdseye` camera provides an initial set of transforms
     * that provide basic rotation, position and scaling support. This is then
     * further adjusted by the 3D transforms held by this camera.
     */
    struct target3dxy {
        /// ### Set up parameters
        struct parameters {
            /// #### Offset
            /**
             * Offset the camera by this before adjusting it's view direction
             * etc. Used to give a more balanced view direction when the camera
             * is finally rotated using the `view_angle`. The offset is applied
             * before the rotation.
             */
            affine::point3d view_offset = {0, 0, 0};
            /// #### View direction unit vector
            affine::point3d view_direction = {0, 0, -1};
            /// #### The distance away from the target
            float distance = {3};
            /// #### View angle
            /**
             * This is how oblique the camera is to looking at the ground.
             *
             * The angle is the rotation around the y-axis where zero means that
             * the camera is pointing straight down.
             */
            float view_angle = {-0.25f * 1 / 3};
            /// #### Perspective transform scaling
            float perspective_scale = 3.333f;
            /// #### Perspective field of view
            /// The units for this are as a multiple of 90°.
            float perspective_fov = 1.0f;
        };


        /// ### Construction
        target3dxy() : target3dxy(parameters{}) {}
        target3dxy(parameters const &);


        /// ### Initial transformation
        orthogonal_birdseye initial;


        /// ### Targetting
        parameters target = {}, current = target;


        /// ### Current matrix transforms
        affine::transform3d view;

        /// ### Return the transform for the current parameters
        affine::transform3d perspective;

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
         *      &planet::camera::target3dxy::updates,
         *      std::ref(warden));
         * ```
         */
        felspar::coro::task<void> updates(felspar::io::warden &);


      private:
        void tick_update();
    };


}
