#pragma once


#include <string>
#include <string_view>


namespace planet::telemetry {


    /// ## Item IDs/names
    /**
     * If the application provides a name then it is used in conjunction with
     * the machine generated name. In any case a unique machine generated name
     * is always provided (unless opted out of using `suffix::no`).
     *
     * If you want an ID for your own type, sub-class (privately) from
     * `planet::telemetry::id` and optionally pass on the user requested name.
     */
    class id {
        std::string m_name;


      public:
        /// ### Choose a suffix policy
        enum class suffix { yes, no };


        /// ### Construction
        explicit id();
        explicit id(char const *n, suffix const s = suffix::yes)
        : id{std::string_view{n}, s} {}
        explicit id(std::string_view const n, suffix const s = suffix::yes)
        : id{std::string{n}, s} {}
        explicit id(std::string, suffix = suffix::yes);

        id(id const &) = delete;
        id(id &&) = default;
        id &operator=(id const &) = delete;
        id &operator=(id &&) = default;


        /// ### The name for this game item
        std::string const &name() const noexcept { return m_name; }
    };


}
