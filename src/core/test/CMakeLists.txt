project(awen-core-test)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    Awen.test.cpp
    Engine.test.cpp
    Object.test.cpp
    ObjectPtr.test.cpp
    Signal.test.cpp
)

enable_testing()
    
find_package(GTest CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    awen-core
    GTest::gtest
    GTest::gtest_main
)

gtest_discover_tests(${PROJECT_NAME})