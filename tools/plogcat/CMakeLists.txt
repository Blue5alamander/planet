add_executable(plogcat
        plogcat.cpp
    )
target_link_libraries(plogcat PUBLIC planet)
install(TARGETS plogcat EXPORT plogcat RUNTIME DESTINATION bin)
