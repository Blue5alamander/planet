#include <planet/folders.hpp>
#include <felspar/test.hpp>

#include <cstdlib>


namespace {


    void set_home(char const *const value) {
#ifdef _WIN32
        ::_putenv_s("APPDATA", value);
#else
        ::setenv("HOME", value, 1);
#endif
    }


    auto const suite = felspar::testsuite(
            "folders",
            /**
             * The base storage folder is the platform's home directory with a
             * platform specific suffix appended
             */
            [](auto check) {
                set_home("/test/home");
                auto const base = planet::base_storage_folder();
#ifdef _WIN32
                check(base) == std::filesystem::path{"/test/home"};
#elif defined(__APPLE__)
                check(base)
                        == std::filesystem::path{"/test/home"}
                                / "Library/Application Support";
#else
                check(base)
                        == std::filesystem::path{"/test/home"} / ".local/share";
#endif
            });


}
