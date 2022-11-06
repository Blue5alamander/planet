add_library(planet
        asset_manager.cpp
        stdin.cpp
    )
target_include_directories(planet PUBLIC ../include/)
target_link_libraries(planet felspar-coro)
