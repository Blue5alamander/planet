add_executable(serialisation-explorer explorer.cpp)
target_link_libraries(serialisation-explorer planet)
install(TARGETS serialisation-explorer EXPORT serialisation-explorer RUNTIME DESTINATION bin)
