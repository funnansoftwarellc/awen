project(awen-flecs)

find_package(flecs CONFIG REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
    include
)

target_link_libraries(${PROJECT_NAME} INTERFACE
    $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>
)
