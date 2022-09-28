add_library(planet
        stdin.cpp
    )
target_include_directories(planet PUBLIC ../include/)

add_executable(cmd-client cmd-client.cpp)
target_link_libraries(cmd-client planet)
