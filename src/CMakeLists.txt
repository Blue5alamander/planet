add_library(planet
        files-audio.cpp
        gain.cpp
        gravity.ui.cpp
        image.cpp
        mixer.cpp
        oscillator.cpp
        scale.ui.cpp
        stdin.cpp
    )
target_include_directories(planet PUBLIC ../include/)
target_link_libraries(planet felspar-io)
