add_library(planet
        connection.cpp
        stdin.cpp
    )
target_include_directories(planet PUBLIC ../include/)
target_link_libraries(planet felspar-coro)
