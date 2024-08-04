#pragma once


#include <planet/serialise/forward.hpp>
#include <planet/telemetry/id.hpp>

#include <map>


namespace planet::telemetry {


    /// ## One of any number of performance counters
    class performance : public id {
      public:
        performance(std::string_view);
        virtual ~performance();


        /// ### Saving and loading
        /**
         * Games that wish to save and load performance counters into game save
         * files or configuration files etc. should use the global
         * `save_performance` and `load_performance` functions instead of these
         * member functions.
         */

        /// #### Save all non-zero performance counters
        static std::size_t current_values(serialise::save_buffer &);
        /**
         * Values are saved into the buffer without a length prefix. The
         * returned value is the number of items that were written.
         */

        /// #### Save this performance counter
        [[nodiscard]] virtual bool save(serialise::save_buffer &) = 0;
        /**
         * Returns `true` if the performance counter has been saved to the
         * buffer.
         *
         * Performance counters are always saved into a box whose name gives the
         * type of counter. The first field in the box is always being the
         * counter's name. The following data is the measurements and the format
         * is determined by the box name.
         */

        /// #### Load performance counters
        using measurements = std::map<std::string, serialise::box>;
        static measurements saved_measurements(serialise::load_buffer &);
        /**
         * The lifetime of the underlying storage for the `load_buffer` passed
         * in must exceed the lifetime of the returned map as the boxes do not
         * own the underlying memory for their content.
         */

        /// #### Load this performance counter's measurements
        [[nodiscard]] virtual bool load(measurements &) = 0;
        /**
         * The measurements structure contains the `box`es for many saved
         * performance counters which may be loaded from. `true` is returned if
         * the `measurements` did include performance data for the this
         * `performance` instance, otherwise `false` is returned.
         *
         * How the loaded measurement data is merged into any existing
         * measurement data depends on the type of the performance counter.
         *
         * When data is loaded for the performance counter then the box
         * containing the measurement data is used up.
         */
    };


}
