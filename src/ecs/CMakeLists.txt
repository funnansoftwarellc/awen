project(awen-ecs)

find_package(flecs CONFIG REQUIRED)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Ecs.ixx
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>
)
