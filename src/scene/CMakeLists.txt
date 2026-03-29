project(awen-scene)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Transform.ixx
        TextureId.ixx
        TextureCache.ixx
        SceneNodes.ixx
        Scene.ixx
)

find_package(flecs CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        awen-graphics
        $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>
)

if(NOT ANDROID)
    add_subdirectory(test)
endif()
