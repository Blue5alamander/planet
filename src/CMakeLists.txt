add_library(planet
        connection.cpp
        stdin.cpp
    )
target_include_directories(planet PUBLIC ../include/)
target_link_libraries(planet felspar-coro)

add_executable(cmd-client cmd-client.cpp)
target_link_libraries(cmd-client planet)
