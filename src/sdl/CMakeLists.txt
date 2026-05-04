project(awen-sdl)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Color.ixx
        Drawables.ixx
        Input.ixx
        Module.ixx
        Phases.ixx
        Renderer.ixx
        Resources.ixx
        Singletons.ixx
        Transform.ixx
        Window.ixx
)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        awen-core
        awen-flecs
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
        glm::glm
)