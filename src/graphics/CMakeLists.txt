project(awen-graphics)

find_package(magic_enum CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Event.ixx
        Color.ixx
        DrawList.ixx
        TextureId.ixx
        TextureCache.ixx
        SceneNodes.ixx
        TraversalPass.ixx
        Window.ixx
        Renderer.ixx
        Graphics.ixx
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        awen-core
        awen-scene
        magic_enum::magic_enum
        raylib
)

if(NOT ANDROID)
    add_subdirectory(test)
endif()

if(EMSCRIPTEN)
    target_link_options(${PROJECT_NAME}
        PUBLIC
            -sUSE_GLFW=3
    )
elseif(ANDROID)
    find_library(log-lib log)
    find_library(android-lib android)
    find_library(GLESv2-lib GLESv2)
    find_library(EGL-lib EGL)

    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            ${log-lib}
            ${android-lib}
            ${GLESv2-lib}
            ${EGL-lib}
    )
else()
    find_package(glfw3 CONFIG REQUIRED)

    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            glfw
    )
endif()
