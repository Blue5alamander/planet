#pragma once


#include <string>
#include <string_view>


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
        /// ### Construction
        explicit id();
        explicit id(std::string_view);

        id(id const &) = delete;
        id(id &&) = default;
        id &operator=(id const &) = delete;
        id &operator=(id &&) = default;


        /// ### The name for this game item
        std::string const &name() const noexcept { return m_name; }
    };


}
