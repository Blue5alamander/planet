cmake_minimum_required(VERSION 3.12)
project(planet-audio)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_custom_target(felspar-check)
    set_property(TARGET felspar-check PROPERTY EXCLUDE_FROM_ALL TRUE)
    add_custom_target(planet-examples)
    set_property(TARGET planet-examples PROPERTY EXCLUDE_FROM_ALL TRUE)
    include(requirements.cmake)
endif()

if(NOT TARGET check)
    add_custom_target(check)
endif()

add_subdirectory(examples)
add_subdirectory(src)
