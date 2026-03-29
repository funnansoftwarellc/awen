project(awen-scene-test)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    Scene.test.cpp
)

enable_testing()

find_package(GTest CONFIG REQUIRED)
find_package(flecs CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    awen-scene
    GTest::gtest
    GTest::gtest_main
    $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>
)

gtest_discover_tests(${PROJECT_NAME})