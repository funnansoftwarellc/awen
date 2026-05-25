project(awen-raylib-test)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    Color.test.cpp
    Events.test.cpp
    Node.test.cpp
    Window.test.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        awen-raylib
        awen-test
)

gtest_discover_tests(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)