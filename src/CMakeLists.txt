add_library(planet
        gravity.ui.cpp
        scale.ui.cpp
        stdin.cpp
    )
target_include_directories(planet PUBLIC ../include/)
target_link_libraries(planet felspar-coro)
