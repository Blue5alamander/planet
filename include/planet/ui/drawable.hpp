#pragma once


namespace planet::ui {


    /// ## Drawable items
    template<typename Renderer>
    struct drawable {
        /// ### Draw at the calculated position
        /**
         * On the face of it, this should be `const`, but some UI elements need
         * to upload data to the GPU and the only time they get a `Renderer` to
         * do that with is in the draw call.
         *
         * This probably means that there's a missing initialisation phase that
         * is able to make use of the `Renderer` instance, but for our simple
         * needs right now, this should suffice.
         */
        virtual void draw(Renderer &) = 0;
    };


}
