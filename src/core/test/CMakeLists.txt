project(awen-core-test)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    Object.test.cpp
)

find_package(GTest REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        awen-core
)

gtest_discover_tests(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)