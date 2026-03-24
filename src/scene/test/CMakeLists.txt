project(awen-scene-test)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    NodePool.test.cpp
    HierarchyPool.test.cpp
    Scene.test.cpp
)

enable_testing()

find_package(GTest CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    awen-scene
    GTest::gtest
    GTest::gtest_main
)

gtest_discover_tests(${PROJECT_NAME})