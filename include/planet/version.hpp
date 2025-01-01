#pragma once


#include <planet/serialise/forward.hpp>

#include <compare>
#include <cstdint>
#include <optional>
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


        /// ### Define the version
        /**
         * - The `appid` should be a string used to identify the game. The
         * string is not user facing so can be a quasi-internal code.
         * - If provided, the `appdir` is the folder name for save games, log
         * files etc. If not provided the `appid` is used.
         * - The `semver` is the user facing string that you want to use as a
         * version number. It must consist only of digits and dots with a
         * maximum of three numbers. This value will be parsed to provide the
         * `semver` data structure with the major, minor and patch numbers.
         * - The `build` number, when provided. Ideally this is a monotonically increasing number that goes up each time a release package (or similar) is produced.
         *
         * TODO Also have an overload that takes the `semver` data structure,
         * then the version string can be free form.
         */
        version(std::string_view appid,
                std::string_view semver);
        version(std::string_view appid,
                std::string_view semver,
                std::uint16_t build);
        version(std::string_view appid,
                std::string_view appdir,
                std::string_view semver,
                std::uint16_t build);


        std::string application_id;
        std::string application_folder;
        std::string version_string;

        planet::semver semver;
        std::optional<std::uint16_t> build;
    };
    void save(serialise::save_buffer &, semver const &);
    void load(serialise::box &, semver &);
    void save(serialise::save_buffer &, version const &);
    void load(serialise::box &, version &);


    std::ostream &operator<<(std::ostream &, version const &);
    std::string to_string(version const &);


}
