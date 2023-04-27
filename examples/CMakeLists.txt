if(TARGET planet-examples)
    add_executable(cmd-client cmd-client.cpp)
    target_link_libraries(cmd-client planet)

    add_executable(oggplayer oggplayer.cpp)
    target_link_libraries(oggplayer asound ogg opus planet vorbis)

    add_executable(snake snake.cpp)
    target_link_libraries(snake planet)

    add_executable(tone tone.cpp)
    target_link_libraries(tone PUBLIC asound planet)
endif()
