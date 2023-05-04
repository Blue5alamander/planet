#pragma once


namespace planet::ui {


    /// ## Drawable items
    template<typename Renderer>
    struct drawable {
        /// ### Draw at the calculated position
        void draw_at(Renderer &, affine::point2d const &offset) const;

      protected:
        // virtual void draw(Renderer &, affine::point2d const &offset) const = 0;
    };


}
