cmake_minimum_required(VERSION 3.12)
project(planet)

add_custom_target(check)

add_subdirectory(libs)
add_subdirectory(src)
add_subdirectory(test)
