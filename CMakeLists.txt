cmake_minimum_required(VERSION 3.12)
project(planet)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_custom_target(felspar-check)
    set_property(TARGET felspar-check PROPERTY EXCLUDE_FROM_ALL TRUE)
    include(requirements.cmake)
endif()
add_custom_target(check)

add_subdirectory(examples)
add_subdirectory(src)
add_subdirectory(test)
