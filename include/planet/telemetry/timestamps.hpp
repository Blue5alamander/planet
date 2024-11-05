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
     * serialised along with the other telemetry types.
     */
    class timestamps : public id {
        static constexpr std::string_view box{"_p:t:timestamps"};


        struct stamps {
            static constexpr std::string_view box{"_p:t:timestamps:s"};


            std::chrono::system_clock::time_point first =
                    std::chrono::system_clock::now();
            std::optional<std::chrono::system_clock::time_point> last = first;
        };
        std::map<std::string, stamps, std::less<>> history;


      public:
        timestamps(std::string_view);


        /// ### Changing timestamps
        /// #### Set a timestamp for the provided key
        void set(std::string_view);
        /// #### Unset a timestamp
        void unset(std::string_view);


        /// ### Returns true if the key is set
        bool is_set(std::string_view) const;


        /// ### Serialisation
        friend void save(planet::serialise::save_buffer &, stamps const &);
        friend void load(planet::serialise::box &, stamps &);
        friend void save(planet::serialise::save_buffer &, timestamps const &);
        friend void load(planet::serialise::box &, timestamps &);
    };
    void save(planet::serialise::save_buffer &, timestamps::stamps const &);
    void load(planet::serialise::box &, timestamps::stamps &);
    void save(planet::serialise::save_buffer &, timestamps const &);
    void load(planet::serialise::box &, timestamps &);


}
