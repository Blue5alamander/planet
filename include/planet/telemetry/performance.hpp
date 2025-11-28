#pragma once


#include <planet/serialise/load_buffer.hpp>
#include <planet/telemetry/id.hpp>

#include <map>


namespace planet::telemetry {


    /// ## One of any number of performance counters
    class performance : public id {
      protected:
        performance(std::string_view, std::source_location const &);


      public:
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
        [[nodiscard]] virtual bool save(serialise::save_buffer &) const = 0;
        /**
         * Returns `true` if the performance counter has been saved to the
         * buffer.
         *
         * Performance counters are always saved into a box whose name gives the
         * type of counter. The first field in the box is always the counter's
         * name. The following data is the measurements and the format is
         * determined by the box name.
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


    namespace detail {
        std::size_t save_performance(
                serialise::save_buffer &, std::span<performance const *>);
        std::size_t load_performance(
                serialise::load_buffer &, std::span<performance *>);
    }


    /// ## Loading and saving performance counters
    /**
     * Because of the way performance counters are saved and loaded it is
     * possible to add new counters to these save files at any time. A counter
     * with no saved data is simply not loaded (and counters that still have
     * zeros in them will not be saved either). This means no special effort
     * needs to be taken when adding new counters.
     */

    /// ### Save a number of performance counters
    template<typename... Performance>
    std::size_t save_performance(serialise::save_buffer &sb, Performance &...p) {
        std::array<performance const *, sizeof...(p)> ps{&p...};
        return detail::save_performance(sb, ps);
    }

    /// ### Load the performance counters
    template<typename... Performance>
    std::size_t load_performance(serialise::load_buffer &lb, Performance &...p) {
        std::array<performance *, sizeof...(p)> ps{&p...};
        return detail::load_performance(lb, ps);
    }


    /// ### Implementing load for a performance counter
    /**
     * Typically this is used to load into a new copy of the underlying storage
     * for the counter whose content is then integrated into the current actual
     * values for the counter. This allows counters to be loaded after values
     * have already been recorded into them.
     */
    template<typename... Fields>
    inline bool load_performance_measurement(
            planet::telemetry::performance::measurements &pd,
            std::string const &name,
            std::string_view const box_name,
            Fields &...fields) {
        if (auto d = pd.find(name); d != pd.end()) {
            d->second.check_name_or_throw(box_name);
            d->second.fields(fields...);
            d->second.check_empty_or_throw();
            return true;
        } else {
            return false;
        }
    }


}
