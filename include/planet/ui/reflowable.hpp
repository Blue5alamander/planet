#pragma once


#include <planet/telemetry/id.hpp>
#include <planet/ui/element.hpp>

#include <felspar/exceptions.hpp>


namespace planet::ui {


    /// ## Reflow UI element layout
    struct reflowable : public telemetry::id {
        using constrained_type = constrained2d<float>;


        /// ### Construction
        reflowable(std::string_view n) : id{n} {}


        /// ### Dirty calculation

        /// #### Set this item as dirty
        void set_dirty() {
            m_constraints = {};
            m_position = {};
        }

        /// #### Check item dirtyness
        /**
         * For UI elements that contain sub-elements this must check all
         * sub-elements and return true if any of them do so.
         */
        virtual bool is_dirty() const {
            return not m_constraints.has_value() or not m_position.has_value();
        }


        /// ### Calculated position and constraints
        /// `reflow` and `move_to` must have been called before these are available

        /// #### The size of the item
        affine::extents2d size() { return constraints().extents(); }
        /// #### Does the reflowable contain the given location
        virtual bool contains_global_coordinate(
                affine::point2d const &p,
                felspar::source_location const & =
                        felspar::source_location::current()) const {
            if (m_position) {
                return m_position->contains(p);
            } else {
                return false;
            }
        }
        /// #### The position that it has
        affine::rectangle2d const &position(
                felspar::source_location const &loc =
                        felspar::source_location::current()) const {
            if (m_position) {
                return *m_position;
            } else {
                throw felspar::stdexcept::logic_error{
                        "Reflowable position has not been set\n"
                        "The position for this reflowable `" +
                                name() +
                                "` has not been set. Generally this means that a parent "
                                "hasn't called `move_to` as it was supposed to\n",
                        loc};
            }
        }
        /// #### The constraints that the item has
        constrained_type const &constraints(
                felspar::source_location const &loc =
                        felspar::source_location::current()) const {
            if (m_constraints) {
                return *m_constraints;
            } else {
                throw felspar::stdexcept::logic_error{
                        "Reflowable constraints have not been set\n"
                        "The constraints for this reflowable `" +
                                name() +
                                "` have not been set. Generally this means that a parent "
                                "hasn't called `reflow` as it was supposed to\n",
                        loc};
            }
        }


        /// ### Calculate size

        /// #### Calculation parameters
        struct reflow_parameters {
            constrained_type screen;

            /// ##### Ensure that `r` is within the `screen`
            affine::rectangle2d within(affine::rectangle2d r) const noexcept {
                /**
                 * The calculations here assume that the rectangle `r` isn't too
                 * large to fit inside the `screen` size.
                 */
                if (r.top_left.x() + r.extents.width > screen.width.max()) {
                    r.top_left.x(screen.width.max() - r.extents.width);
                } else if (r.top_left.x() < screen.width.min()) {
                    r.top_left.x(screen.width.min());
                }
                if (r.top_left.y() + r.extents.height > screen.height.max()) {
                    r.top_left.y(screen.height.max() - r.extents.height);
                } else if (r.top_left.y() < screen.height.min()) {
                    r.top_left.y(screen.height.min());
                }
                return r;
            }
        };

        /// #### Size calculation
        /**
         * Calculates the correct position for this element given the
         * constraints and returns a concrete position that the element will be
         * drawn in given the constraints.
         *
         * The constraint passed in represents the possible extents of the item,
         * and the returned constraint gives minimums and maximums for the
         * layout. In general it isn't guaranteed that the input constraint can
         * be met, and parent items need to take this into account or layouts
         * will break in weird and wonderful ways.
         *
         * After calling `reflow` on all children the parent element must
         * determine their locations. Once done the parent element calls
         * `move_to` on each child to inform it of where to draw.
         */
        constrained_type const &
                reflow(reflow_parameters const &p, constrained_type const &ex) {
            m_constraints = do_reflow(p, ex);
            return *m_constraints;
        }


        /// ### Move the widget to a new position
        /**
         * For widgets (or other UI items that have a `panel`) the parent must
         * tell the child what the new position is going to be.
         *
         * Either the widget is to be placed at a given top left position at
         * whatever its natural size is, or it is to be placed at a given
         * location covering the entire rectangle it's told to use.
         */
        affine::rectangle2d
                move_to(reflow_parameters const &p,
                        affine::rectangle2d const &r) {
            constraints(); // Reflow must have been called first
            m_position = move_sub_elements(p, r);
            return *m_position;
        }


      protected:
        /// ### Reflow implementation
        virtual constrained_type do_reflow(
                reflow_parameters const &, constrained_type const &) = 0;


        /// ### Move sub-elements to final positions
        /**
         * Implement this in order to set the positions for any sub-elements
         * that the reflowable may have. Return the rectangle that the
         * `reflowable` covers.
         *
         * For widgets this method name changes slightly. See
         * [widget.hpp](./widget.hpp) for more details.
         */
        virtual affine::rectangle2d move_sub_elements(
                reflow_parameters const &, affine::rectangle2d const &) = 0;

      private:
        std::optional<constrained_type> m_constraints;
        std::optional<affine::rectangle2d> m_position;
    };


}
