#include <planet/folders.hpp>


namespace {
    char const *safe_getenv(char const *env) {
        char const *value = std::getenv(env);
        if (value) {
            return value;
        } else {
            return "";
        }
    }
}


std::filesystem::path planet::base_storage_folder() {
    std::filesystem::path home;
#ifdef _WIN32
    home = safe_getenv("APPDATA");
#else
    home = safe_getenv("HOME");
#endif
    if (home.empty()) {
        home = std::filesystem::current_path();
    } else {
#ifndef _WIN32
        home /= ".local/share";
#endif
    }
    return home;
}
