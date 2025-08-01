find_package(Threads REQUIRED)

add_library(planet
        audio.cpp
        affine.cpp
        asset_manager.cpp
        baseplate.ui.cpp
        cameras.cpp
        comms.cpp
        ecs.cpp
        files.audio.cpp
        files.drawing.cpp
        folders.cpp
        gravity.ui.cpp
        image.drawing.cpp
        log.cpp
        map.cpp
        mouse.events.cpp
        panel.ui.cpp
        serialise.cpp
        scale.ui.cpp
        stdin.cpp
        telemetry.cpp
        time.cpp
        ui.cpp
        version.cpp
    )
target_compile_features(planet PUBLIC cxx_std_23)
target_include_directories(planet PUBLIC ../include/)
target_compile_definitions(planet PRIVATE
    LOG_ROOT_DIRECTORY="${PLANET_LOG_ROOT_DIRECTORY}")
target_link_libraries(planet
        felspar-io
        felspar-parse
        ogg
        Threads::Threads
        vorbis
    )
install(TARGETS planet LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
