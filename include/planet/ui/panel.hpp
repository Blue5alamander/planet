#pragma once


#include <planet/affine2d.hpp>
#include <planet/events/mouse.hpp>

#include <felspar/coro/bus.hpp>
#include <felspar/coro/eager.hpp>


namespace planet::ui {


    /// ## Co-ordinate transform hierarchy co-ordinate spaces
    class panel final {
        panel *parent = nullptr;

        /// ### Transformation into and out of the local coordinate space
        affine::transform2d viewport = {};

        struct child final {
            std::optional<affine::rectangle2d> area;
            panel *sub;

            child(panel *);
            child(panel *, affine::rectangle2d);
        };
        std::vector<child> children;
        child &add(panel *p);

        // Swap out the current parent with a new one
        void reparent_children(panel *);

      public:
        /// ### Construction and destruction
        panel() = default;
        ~panel();

        /**
         * Given the way we want to be able to use panels, we have to be able to
         * ensure stable addresses, so: not copyable for sure, and never movable
         * once there are children.
         */
        panel(panel const &) = delete;
        panel(panel &&,
              felspar::source_location const & =
                      felspar::source_location::current());
        panel &operator=(panel const &) = delete;
        panel &operator=(panel &&) = delete;


        /// ### Co-ordinate space for this panel

        /// #### Remove all transformations into and out of the coordinate space
        void reset_coordinate_space() { viewport = {}; }

        /// #### In and out of the co-ordinate space
        /**
         * Takes the panel hierarchy into account. When going into the screen
         * co-ordinate space we must apply our local transformations before we
         * apply the parent ones. When coming out of the screen coordinates we
         * have to undo the parent transformation first
         */
        affine::point2d into(affine::point2d const p) {
            if (parent) {
                return parent->into(viewport.into(p));
            } else {
                return viewport.into(p);
            }
        }
        affine::point2d outof(affine::point2d const p) {
            if (parent) {
                return viewport.outof(parent->outof(p));
            } else {
                return viewport.outof(p);
            }
        }
        /// #### Provide access to the matrices
        auto into() const noexcept { return viewport.into(); }
        auto outof() const noexcept { return viewport.outof(); }

        /// #### Is a location in the panel
        /**
         * Returns true if the location (in the screen coordinate space) is
         * within the area of the panel.
         */
        bool contains(affine::point2d const &) const;

        /// #### Transformations of the local co-ordinate space
        panel &reflect_y() {
            viewport.reflect_y();
            return *this;
        }
        panel &translate(affine::point2d const p) {
            viewport.translate(p);
            return *this;
        }
        panel &scale(float const s) {
            viewport.scale(s);
            return *this;
        }
        panel &rotate(float const t) {
            viewport.rotate(t);
            return *this;
        }


        /// ### Panel hierarchy management

        /// #### Tell if this parent currently has a parent or not
        bool has_parent() const noexcept { return parent; }
        /// #### Add to a parent, but don't position it
        void add_child(panel &);
        /// #### Add a child
        /**
         * The child covers the requested portion of the parent in this panel's
         * coordinate space
         */
        void add_child(panel &, affine::rectangle2d);
        void add_child(
                panel &p,
                affine::point2d const top_left,
                affine::point2d const bottom_right) {
            add_child(p, {top_left, bottom_right});
        }
        /// #### Remove the passed child from the panel
        void remove_child(panel &);


        /// ### Panel positioning

        /// #### Move the location of this panel in the parent's space
        void move_to(affine::rectangle2d);


        /// ### Drawing in the panel coordinate space

        /// #### Draw a line between two points in world co-ordinate space
        template<typename R>
        void
                line(R &renderer,
                     affine::point2d const cp1,
                     affine::point2d const cp2) const {
            auto const p1 = viewport.into(cp1);
            auto const p2 = viewport.into(cp2);
            renderer.line(p1.x(), p1.y(), p2.x(), p2.y());
        }
        /// #### Draw the texture
        /// At native size with its top left corner at the location specified
        template<typename R, typename T>
        void copy(R &renderer, T const &tex, affine::point2d const l) const {
            auto const p = viewport.into(l);
            renderer.copy(tex, p.x(), p.y());
        }
        /// #### Scale the texture
        /// The texture fills the rectangle
        template<typename R, typename T>
        void copy(R &renderer, T const &tex, affine::rectangle2d const r) const {
            auto const tl = viewport.into(r.top_left);
            auto const br = viewport.into(r.bottom_right());
            renderer.copy(
                    tex, tl.x(), tl.y(), br.x() - tl.x(), br.y() - tl.y());
        }
    };


}
