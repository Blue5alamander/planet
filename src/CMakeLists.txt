find_package(Threads REQUIRED)

add_library(planet
        affine.cpp
        asset_manager.cpp
        baseplate.ui.cpp
        cameras.cpp
        colour.cpp
        comms.cpp
        ecs.cpp
        files.drawing.cpp
        folders.cpp
        gravity.ui.cpp
        image.drawing.cpp
        log.cpp
        map.cpp
        maths.cpp
        mouse.events.cpp
        panel.ui.cpp
        serialise.cpp
        scale.ui.cpp
        stdin.cpp
        telemetry.cpp
        time.cpp
        ui.cpp
        version.cpp
        vertex.cpp
    )
target_compile_features(planet PUBLIC cxx_std_23)
target_include_directories(planet PUBLIC ../include/)
target_compile_definitions(planet PRIVATE
    LOG_ROOT_DIRECTORY="${PLANET_LOG_ROOT_DIRECTORY}")
target_link_libraries(planet
        felspar-io
        felspar-parse
        Threads::Threads
    )

if(PLANET_NO_AUDIO)
    target_compile_definitions(planet PUBLIC PLANET_NO_AUDIO)
else()
    target_sources(planet PRIVATE
            audio.cpp
            files.audio.cpp
        )
    target_link_libraries(planet
            FLAC
            ogg
            vorbis
        )
endif()
install(TARGETS planet LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


add_test_run(check planet TESTS
        affine2d.tests.cpp
        affine3d.tests.cpp
        asset_manager.tests.cpp
        baseplate.ui.tests.cpp
        box.layout.tests.cpp
        breakable_row.layout.tests.cpp
        cameras.tests.cpp
        column.layout.tests.cpp
        constrained.tests.cpp
        entities.tests.cpp
        event.tests.cpp
        gravity.tests.cpp
        grid.layout.tests.cpp
        headers.cpp
        log.tests.cpp
        map.hex.tests.cpp
        map.square.tests.cpp
        maths.tests.cpp
        panel.tests.cpp
        performance.tests.cpp
        range.ui.tests.cpp
        row.layout.tests.cpp
        serialise.tests.cpp
        serialise.variant.tests.cpp
        scaling.tests.cpp
        target_aspect.layout.tests.cpp
        target_size.layout.tests.cpp
        telemetry.tests.cpp
        time.tests.cpp
        version.tests.cpp
    )
if(NOT PLANET_NO_AUDIO)
    add_test_run(check planet TESTS
            audio.tests.cpp
            mixer.tests.cpp
        )
endif()
