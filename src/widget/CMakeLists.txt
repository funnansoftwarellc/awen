project(awen-widget)


add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        Color.ixx
        Event.ixx
        Module.ixx
        Node.ixx
        NodePolygon.ixx
        NodeTransform.ixx
        NodeCircle.ixx
        NodeRectangle.ixx
        NodeText.ixx
        Components.ixx
        Window.ixx
)

find_package(glm CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC
    awen-core
    awen-sdl
    glm::glm
)

# if(NOT ANDROID)
#     add_subdirectory(test)
# endif()