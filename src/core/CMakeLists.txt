project(awen-core)


add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Engine.ixx
        Event.ixx
        Module.ixx
        Object.ixx
        ObjectPtr.ixx
        Overloaded.ixx
        Signal.ixx
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(Threads REQUIRED)
find_package(SDL3 CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC
    awen-flecs
    Threads::Threads
    SDL3::SDL3
)

if(NOT ANDROID)
    add_subdirectory(test)
endif()