if(TARGET planet-examples)
    add_executable(cmd-client cmd-client.cpp)
    target_link_libraries(cmd-client planet)

    add_executable(snake snake.cpp)
    target_link_libraries(snake planet)
endif()
