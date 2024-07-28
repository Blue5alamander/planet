cmake_minimum_required(VERSION 3.12)
project(planet)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_custom_target(felspar-check)
    set_property(TARGET felspar-check PROPERTY EXCLUDE_FROM_ALL TRUE)
    add_custom_target(felspar-stress)
    set_property(TARGET felspar-stress PROPERTY EXCLUDE_FROM_ALL TRUE)
    add_custom_target(planet-examples)
    set_property(TARGET planet-examples PROPERTY EXCLUDE_FROM_ALL TRUE)

    include(requirements.cmake)
    add_compile_options(
            -Werror
            -fdiagnostics-color=always
            -fdiagnostics-show-option
            -Wall -Wextra -Wpedantic
        )
endif()

if(NOT TARGET check)
    add_custom_target(check)
endif()

add_subdirectory(examples)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(tools)
add_subdirectory(tooling)

target_compile_definitions(felspar-coro INTERFACE FELSPAR_CORO_SKIP_LIFETIME_CHECKS)
