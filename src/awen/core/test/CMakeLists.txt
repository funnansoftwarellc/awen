project(awen-core-test)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
        TimerFixed.test.cpp
)

find_package(GTest CONFIG REQUIRED)
find_package(Qt6 ${AWEN_QT_VERSION} CONFIG REQUIRED COMPONENTS Core Test)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        awen-core
        GTest::gtest
        Qt6::Core
        Qt6::Test
)

gtest_discover_tests(${PROJECT_NAME})
