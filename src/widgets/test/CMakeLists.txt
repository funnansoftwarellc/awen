project(awen-widgets-test)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    Widgets.test.cpp
)

enable_testing()

find_package(GTest CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    awen-widgets
    GTest::gtest
    GTest::gtest_main
)

gtest_discover_tests(${PROJECT_NAME})
