project(awen-widgets)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    TextureCache.ixx
        Widgets.ixx
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        awen-flecs
        awen-scene
        awen-graphics
)

if(NOT ANDROID)
    add_subdirectory(test)
endif()
