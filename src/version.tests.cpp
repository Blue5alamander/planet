#include <planet/version.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite(
            "version",
            [](auto check) {
                planet::version v{"appid", "0.0", 3};
                check(v.application_id) == "appid";
                check(v.application_folder) == "appid";
                check(v.version_string) == "0.0";
                check(v.semver.major) == 0;
                check(v.semver.minor) == 0;
                check(v.semver.patch) == 0;
                check(v.build) == 3;
                check(v.git_describe) == "";
            },
            [](auto check) {
                planet::version v{"appid2", "0.2", 53};
                check(v.application_id) == "appid2";
                check(v.application_folder) == "appid2";
                check(v.version_string) == "0.2";
                check(v.semver.major) == 0;
                check(v.semver.minor) == 2;
                check(v.semver.patch) == 0;
                check(v.build) == 53;
                check(v.git_describe) == "";
            },
            [](auto check) {
                planet::version v{"appid2", "5.26.123", 2253};
                check(v.application_id) == "appid2";
                check(v.application_folder) == "appid2";
                check(v.version_string) == "5.26.123";
                check(v.semver.major) == 5;
                check(v.semver.minor) == 26;
                check(v.semver.patch) == 123;
                check(v.build) == 2253;
                check(v.git_describe) == "";
            },
            [](auto check) {
                planet::version v{"appid2", "folder", "5.26.123", 2253, "5.25-45-g123456-dirty"};
                check(v.application_id) == "appid2";
                check(v.application_folder) == "folder";
                check(v.version_string) == "5.26.123";
                check(v.semver.major) == 5;
                check(v.semver.minor) == 26;
                check(v.semver.patch) == 123;
                check(v.build) == 2253;
                check(v.git_describe) == "5.25-45-g123456-dirty";
            });


}
