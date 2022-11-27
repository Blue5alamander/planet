if(TARGET planet-examples)
    add_executable(tone tone.cpp)
    target_link_libraries(tone PUBLIC asound planet-audio)
endif()
