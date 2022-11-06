add_library(planet-audio
        mixer.cpp
        oscillator.cpp
    )
target_include_directories(planet-audio PUBLIC ../include)
target_link_libraries(planet-audio PUBLIC felspar-io)
