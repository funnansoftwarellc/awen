project(awen-sdl)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Module.ixx
        Color.ixx
        Event.ixx
        DrawList.ixx
        Font.ixx
        Renderer.ixx
        Window.ixx
)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        awen-core
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
)