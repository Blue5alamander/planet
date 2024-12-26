#pragma once


#include <planet/serialise/forward.hpp>

#include <compare>
#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>


namespace planet {


    struct semver {
        static constexpr std::string_view box{"_p:semver"};


        std::uint16_t major, minor, patch;


        friend auto operator<=>(semver const &, semver const &) = default;
    };


    struct version {
        static constexpr std::string_view box{"_p:version"};


        version(std::string_view appid,
                std::string_view semver,
                std::uint16_t build);


        std::string application_id;
        std::string version_string;

        planet::semver semver;
        std::uint16_t build;
    };
    void save(serialise::save_buffer &, semver const &);
    void load(serialise::box &, semver &);
    void save(serialise::save_buffer &, version const &);
    void load(serialise::box &, version &);


    std::ostream &operator<<(std::ostream &, version const &);
    std::string to_string(version const &);


}
