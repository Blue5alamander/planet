#pragma once


#include <planet/serialise/forward.hpp>
#include <planet/telemetry/id.hpp>

#include <chrono>
#include <map>
#include <string>


namespace planet::telemetry {


    /**
     * ## Key/timestamp storage
     *
     * Stores timestamps when a particular key has been presented to the user.
     * Designed for tutorial hints and "don't show this again" checkboxes on
     * dialogue boxes.
     *
     * The stored timestamps are not performance data, so they are not
     * serialised along with the other telemetry types -- they must be
     * serialised as normal data fields.
     *
     * **NB**: This type is **not** thread safe
     */
    class timestamps : public id {
      public:
        static constexpr std::string_view box{"_p:t:timestamps"};


        struct stamps {
            static constexpr std::string_view box{"_p:t:timestamps:s"};


            std::chrono::system_clock::time_point first =
                    std::chrono::system_clock::now();
            std::optional<std::chrono::system_clock::time_point> last;


            friend bool operator==(stamps const &, stamps const &) noexcept =
                    default;
        };


        timestamps(std::string_view);


        /// ### Changing timestamps

        /// #### Set a timestamp for the provided key
        void set(std::string_view);
        /**
         * If the key has not previously been set then a new `stamp` is
         * recorded. If the key has previously been set then only the
         * `stamp::last` time is updated.
         */

        /// #### Unset a timestamp
        void unset(std::string_view);


        /// ### Returns true if the key is set
        bool is_set(std::string_view) const;


        /// ### Return the time information for the key
        std::optional<stamps> times_for(std::string_view) const;


        /// ### Serialisation
        friend void save(planet::serialise::save_buffer &, stamps const &);
        friend void load(planet::serialise::box &, stamps &);
        friend void save(planet::serialise::save_buffer &, timestamps const &);
        friend void load(planet::serialise::box &, timestamps &);


      private:
        std::map<std::string, stamps, std::less<>> history;
    };
    void save(planet::serialise::save_buffer &, timestamps::stamps const &);
    void load(planet::serialise::box &, timestamps::stamps &);
    void save(planet::serialise::save_buffer &, timestamps const &);
    void load(planet::serialise::box &, timestamps &);


}
