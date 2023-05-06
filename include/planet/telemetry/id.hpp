#pragma once


#include <string>


namespace planet::telemetry {


    /// ## Item IDs/names
    /**
     * If the application provides a name then it is used in conjunction with
     * the machine generated name. In any case a unique machine generated name
     * is always provided.
     *
     * If you want an ID for your own type, sub-class from
     * `planet::telemetry::id` and optionally pass on the user requested name
     * and optionally the sub-class type ID that you want to make use of.
     */
    class id {
        std::string m_name;

      public:
        id();

        /// ### The name for this game item
        std::string const &name() const noexcept { return m_name; }
    };


}
