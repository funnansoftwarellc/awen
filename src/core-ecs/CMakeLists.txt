project(awen-core-ecs)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Engine.ixx
        Module.ixx
        Phases.ixx
)

find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Threads::Threads
        awen-flecs
)

if(NOT ANDROID AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
    add_subdirectory(test)
endif()