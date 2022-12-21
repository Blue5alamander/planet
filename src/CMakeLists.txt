add_library(planet
        asset_manager.cpp
        files.audio.cpp
        files.drawing.cpp
        gain.audio.cpp
        gravity.ui.cpp
        image.drawing.cpp
        mixer.audio.cpp
        mouse.events.cpp
        oscillator.audio.cpp
        scale.ui.cpp
        stdin.cpp
    )
target_include_directories(planet PUBLIC ../include/)
target_link_libraries(planet felspar-io)
